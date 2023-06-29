#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <syncstream>
#include <chrono>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace sys = boost::system;
namespace net = boost::asio;

using namespace std::chrono;
using namespace std::literals;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class ThreadChecker {
public:
    explicit ThreadChecker(std::atomic_int& counter)
        : counter_{counter} {
    }

    ThreadChecker(const ThreadChecker&) = delete;
    ThreadChecker& operator=(const ThreadChecker&) = delete;

    ~ThreadChecker() {
        // assert выстрелит, если между вызовом конструктора и деструктора
        // значение expected_counter_ изменится
        assert(expected_counter_ == counter_);
    }

private:
    std::atomic_int& counter_;
    int expected_counter_ = ++counter_;
};

class Logger {
public:
    explicit Logger(std::string id)
        : id_(std::move(id)) {
    }

    template <typename... Args>
    void LogMessage(Args ...args) const {
        std::osyncstream os{std::cout};
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
           << "s] "sv;
        ((os << args), ...);
        os << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
};

class Order : public std::enable_shared_from_this<Order> {
public:
    Order(
        net::io_context& io, int id, HotDogHandler handler, std::shared_ptr<GasCooker> gas_cooker,
        std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread
    )
        : io_{io}, id_{id}, handler_{std::move(handler)}, gas_cooker_(std::move(gas_cooker)), sausage_(std::move(sausage)), bread_(std::move(bread)) { }

    // Запускает асинхронное выполнение заказа
    void Execute() {
        BakeBread();
        FrySausage();
    }

private:
    void BakeBread() {
        logger_.LogMessage("Start baking bread");
        bread_->StartBake(*gas_cooker_, [self = shared_from_this()]() {
            self->bread_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnBaked(ec);
            }));
        });
    }

    void OnBaked(sys::error_code ec) {
        ThreadChecker checker{counter_};
        bread_->StopBake();
        if (ec) {
            logger_.LogMessage("Bake error : "s + ec.what());
        } else {
            logger_.LogMessage(
                "Breed has been baked in "sv,
                std::chrono::duration_cast<std::chrono::duration<double>>(bread_->GetBakingDuration()).count(),
                " seconds"
            );
            bread_baked_ = true;
        }
        CheckReadiness();
    }

    void FrySausage() {
        logger_.LogMessage("Start baking bread");
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()]() {
            self->sausage_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnFried(ec);
            }));
        });
    }

    void OnFried(sys::error_code ec) {
        ThreadChecker checker{counter_};
        sausage_->StopFry();
        if (ec) {
            logger_.LogMessage("Fry error : "s + ec.what());
        } else {
            logger_.LogMessage(
                "Sausage has been fried in "sv,
                std::chrono::duration_cast<std::chrono::duration<double>>(sausage_->GetCookDuration()).count(),
                " seconds"
            );
            sausage_fried_ = true;
        }
        CheckReadiness();
    }

    void CheckReadiness() {
        if (delivered_) {
            return;
        }

        // Если все компоненты гамбургера готовы, упаковываем его
        if (IsReadyToDeliver()) {
            Deliver();
        }
    }

    void Deliver() {
        delivered_ = true;
        handler_(Result{HotDog{id_, sausage_, bread_}});
    }

    bool IsReadyToDeliver() const {
        return bread_baked_ && sausage_fried_ &&
               sausage_->IsCooked() && bread_->IsCooked();
    }

    int id_;
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    std::shared_ptr<GasCooker> gas_cooker_;
    HotDogHandler handler_;
    Logger logger_{std::to_string(id_)};
    std::shared_ptr<Sausage> sausage_; 
    std::shared_ptr<Bread> bread_;
    bool bread_baked_ = false, sausage_fried_ = false, delivered_ = false;
    net::steady_timer bread_timer_{io_, Milliseconds{1000}};
    net::steady_timer sausage_timer_{io_, Milliseconds{1500}};
    std::atomic_int counter_{0};
};

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // 1) Выпекаем булку в течение 1 секунды, жарим сосиску в течение 1.5 секунд
        // 2) Собираем из них хот-дог
        const int order_id = ++next_order_id_;
        std::make_shared<Order>(
            io_, order_id, std::move(handler), gas_cooker_, store_.GetSausage(), store_.GetBread()
        )->Execute();
    }

private:
    net::io_context& io_;
    std::atomic_int next_order_id_ = 0;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};
