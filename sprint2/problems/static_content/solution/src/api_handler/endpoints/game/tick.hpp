#pragma once

#include "api_handler/endpoints/endpoint.hpp"
#include <chrono>

class TickEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        if (game_.GetTickPeriod().has_value())
            return model::api::errors::invalid_endpoint();

        try {
            auto [timedelta] = value_to<model::api::requests::TickRequest>(boost::json::parse(request.body()));
            return execute(timedelta);
        } catch (...) {
            return model::api::errors::parse_error();
        }
    }
    util::Response execute(std::chrono::milliseconds timedelta) {
        game_.Tick(timedelta.count());
        return responses::ok();
    }

  private:
    struct responses {
        static util::Response ok() { return util::Response::Json(http::status::ok, json::value()).no_cache(); }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/tick"};
};