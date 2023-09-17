#include "util/sdk.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include "http_server.hpp"
#include "json_loader.hpp"
#include "request_handler.hpp"
#include "util/logging.hpp"
#include "util/ticker.hpp"

using namespace std::literals;
using namespace util;

namespace net = boost::asio;
namespace logging = boost::log;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned num_threads, const Fn &fn) {
    num_threads = std::max(1u, num_threads);
    std::vector<std::jthread> workers;
    workers.reserve(num_threads - 1);
    // Запускаем num_threads-1 рабочих потоков, выполняющих функцию fn
    while (--num_threads) {
        workers.emplace_back(fn);
    }
    fn();
}

} // namespace

struct Args {
    std::optional<int> tick_period;
    std::string config_file;
    std::string www_root;
    bool randomize_spawn_points{false};
};

[[nodiscard]]
std::optional<Args> ParseCommandLine(int argc, const char *const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"All options"s};

    Args args;
    // clang-format off
    int tick_period;
    desc.add_options()
        ("help,h", "Show help")
        ("tick-period,t", po::value(&tick_period)->value_name("milliseconds"s), "set tick period")
        ("config-file,c", po::value(&args.config_file)->value_name("file"), "set config file path")
        ("www-root,w", po::value(&args.www_root)->value_name("dir"), "set static files root")
        ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_points), "spawn dogs at random positions");
    // clang-format on

    // variables_map хранит значения опций после разбора
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        // Если был указан параметр --help, то выводим справку и возвращаем nullopt
        std::cout << desc;
        return std::nullopt;
    }

    if (vm.contains("tick-period")) {
        args.tick_period = tick_period;
    }

    if (!vm.contains("config-file")) {
        throw std::runtime_error{"Config file has not been specified"s};
    }

    if (!vm.contains("www-root")) {
        throw std::runtime_error{"Static files path has not been specified"s};
    }

    // С опциями программы всё в порядке, возвращаем структуру args
    return args;
}

int main(int argc, const char *argv[]) {
    // 0. Инициализируем логер
    logging::add_console_log(std::clog, logging::keywords::format = &LogFormatter);
    logging::add_common_attributes();

    try {
        auto args = ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_SUCCESS;
        }

        // 1. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        auto api_strand = net::make_strand(ioc);

        // 2. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code &ec, int signal_number) {
            if (!ec) {
                ioc.stop();
            }
        });

        // 3. Загружаем карту из файла и строим модель игры
        model::Game game = json_loader::LoadGame(args->config_file);
        game.SetRandomizeSpawnPoint(args->randomize_spawn_points);
        if (args->tick_period) {
            game.SetTickPeriod(*args->tick_period);
            Ticker ticker{api_strand, std::chrono::milliseconds{*args->tick_period},
                          [tick_period = *args->tick_period, &game](std::chrono::milliseconds delta) {
                              game.Tick(delta.count());
                          }};
            ticker.Start();
        }

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        request_handler::RequestHandler handler{game, args->www_root, api_strand};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&handler](auto &&addr, auto &&req, auto &&send) {
            handler(std::forward<decltype(addr)>(addr), std::forward<decltype(req)>(req),
                    std::forward<decltype(send)>(send));
        });

        // 6. Запускаем обработку асинхронных операций
        LogStart(address.to_string(), port);
        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

        // Логирование успешного завершения программы
        LogExit(EXIT_SUCCESS);

        return EXIT_SUCCESS;
    } catch (const std::exception &ex) {
        // Логирование завершения программы с ошибкой
        LogExit(EXIT_FAILURE, ex.what());
        return EXIT_FAILURE;
    }
}
