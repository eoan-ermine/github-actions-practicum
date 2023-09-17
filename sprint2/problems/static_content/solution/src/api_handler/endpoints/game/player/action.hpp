#pragma once

#include "api_handler/endpoints/endpoint.hpp"
#include "model/domains/api.hpp"

class ActionEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        auto method = request.method();

        // TODO: Пустой токен засчитывается за валидный, исправить
        if (!request.count("Authorization") || !request["Authorization"].starts_with("Authorization: Bearer ")) {
            return model::api::errors::no_token();
        } else if (method != http::verb::get && method != http::verb::head) {
            return model::api::errors::only_post();
        } else if (!request.count("Content-Type") || request["Content-Type"] != "application/json") {
            return model::api::errors::invalid_content_type();
        }

        try {
            auto [direction] = value_to<model::api::requests::ActionRequest>(boost::json::parse(request.body()));
            return execute(direction, request["Authorization"]);
        } catch (...) {
            return model::api::errors::parse_error();
        }
    }
    util::Response execute(model::Direction direction, std::string token) {
        auto player = game_.GetPlayer(token);
        if (!player) {
            return model::api::errors::no_user_found();
        }

        auto s = player->GetSession()->GetMap()->GetDogSpeed().value();
        switch (direction) {
        case model::Direction::NORTH:
            player->GetDog()->SetSpeed({0, -s});
            break;
        case model::Direction::SOUTH:
            player->GetDog()->SetSpeed({0, s});
            break;
        case model::Direction::WEST:
            player->GetDog()->SetSpeed({-s, 0});
            break;
        case model::Direction::EAST:
            player->GetDog()->SetSpeed({0, s});
            break;
        case model::Direction::NO:
            player->GetDog()->SetSpeed({0, 0});
            break;
        }

        return responses::ok();
    }

  private:
    struct responses {
        static util::Response ok() { return util::Response::Json(http::status::ok, json::value()).no_cache(); }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/player/action"};
};