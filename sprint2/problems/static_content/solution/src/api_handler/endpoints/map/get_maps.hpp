#pragma once

#include "api_handler/endpoints/endpoint.hpp"

class GetMapsEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override { return responses::ok(game_); }

  private:
    struct responses {
        static util::Response ok(model::Game &game_) {
            return util::Response::Json(http::status::ok, json::value_from(game_.GetMaps()));
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/maps"};
};