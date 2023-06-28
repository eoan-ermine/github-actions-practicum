// Подключаем заголовочный файл <sdkddkver.h> в системе Windows,
// чтобы избежать предупреждения о неизвестной версии Platform SDK,
// когда используем заголовочные файлы библиотеки Boost.Asio
#ifdef WIN32
#include <sdkddkver.h>
#endif

// Boost.Beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <optional>
#include <iostream>
#include <thread>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;

namespace beast = boost::beast;
namespace http = beast::http;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer) {
    beast::error_code ec;
    StringRequest request;
    
    // Считываем из socket запрос request, используя buffer для хранения данных.
    // В ec функция запишет код ошибки.
    http::read(socket, buffer, request, ec);

    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }
    if (ec) {
        throw std::runtime_error("Failed to read request: "s.append(ec.message()));
    }

    return request;
}

void DumpRequest(const StringRequest& request) {
    std::cout << request.method_string() << ' ' << request.target() << std::endl;
    // Выводим заголовки запроса
    for (const auto& header : request) {
        std::cout << "  "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
}

// Структура ContentType задаёт область видимости для констант,
// задающий значения HTTP-заголовка Content-Type
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse MakeEmptyResponse(http::status status, std::string_view content_length, unsigned http_version,
                                 bool keep_alive,
                                 std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::content_length, content_length);
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HandleRequest(StringRequest&& request) {
    auto text_response = [&request](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, request.version(), request.keep_alive());
    };
    auto empty_response = [&request](http::status status) {
        return MakeStringResponse(status, request["Content-Length"], request.version(), request.keep_alive());
    };

    switch(request.method()) {
    case http::verb::get:
        return text_response(
            http::status::ok,
            "Hello, "s.append(request.target().substr(1))
        );
    case http::verb::head:
        return empty_response(http::status::ok);
    default:
        StringResponse response = text_response(http::status::method_not_allowed, "Invalid method"sv);
        response.set("Allow"sv, "GET, HEAD"sv);
        return response;
    }
}

template <typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& handle_request) {
    try {
        // Буфер для чтения данных в рамках текущей сессии.
        beast::flat_buffer buffer;

        // Продолжаем обработку запросов, пока клиент их отправляет
        while (auto request = ReadRequest(socket, buffer)) {
            DumpRequest(*request);

            StringResponse response = handle_request(*std::move(request));
            // Отправляем ответ сервера клиенту
            http::write(socket, response);

            // Прекращаем обработку запросов, если семантика ответа требует это
            if (response.need_eof()) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    beast::error_code ec;
    // Запрещаем дальнейшую отправку данных через сокет
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    // Контекст для выполнения синхронных и асинхронных операций ввода/вывода
    net::io_context ioc;

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 8080;

    // Объект, позволяющий принимать tcp-подключения к сокету
    tcp::acceptor acceptor(ioc, {address, port});

    std::cout << "Server has started..."sv << std::endl;
    while (true) {
        std::cout << "Waiting for socket connection"sv << std::endl;
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        std::cout << "Connection received"sv << std::endl;

        // Запускаем обработку взаимодействия с клиентом в отдельном потоке
        std::thread t(
            // Лямбда-функция будет выполняться в отдельном потоке
            [](tcp::socket socket) {
                HandleConnection(socket, HandleRequest);
            },
            std::move(socket));  // Сокет нельзя скопировать, но можно переместить

        // После вызова detach поток продолжит выполняться независимо от объекта t
        t.detach();
    }
}