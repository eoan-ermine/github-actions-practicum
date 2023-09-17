#pragma once

#include "api_handler/endpoints/endpoint.hpp"
#include <string_view>

class GetPlayersEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override { return request.target() == endpoint; }
    util::Response handle(const http::request<http::string_body> &request) override {
        auto method = request.method();

        // TODO: Пустой токен засчитывается за валидный, исправить
        std::string_view authorization_prefix = "Bearer ";
        if (!request.count("Authorization") || !request["Authorization"].starts_with(authorization_prefix)) {
            return model::api::errors::no_token();
        } else if (method != http::verb::get && method != http::verb::head) {
            return model::api::errors::only_get_and_head();
        } else {
            std::string_view token = request["Authorization"];
            token.remove_prefix(authorization_prefix.size());
            return execute(token);
        }
    }
    util::Response execute(std::string_view token) {
        auto player = game_.GetPlayer(token);
        if (!player) {
            return model::api::errors::no_user_found();
        }
        const auto &players = game_.GetPlayers(player->GetSession().GetMap().GetId());
        return responses::ok(players);
    }

  private:
    struct responses {
        static util::Response ok(const std::unordered_map<model::Player::Id, std::shared_ptr<model::Player>> &players) {
            return util::Response::Json(http::status::ok,
                                        json::value_from(model::api::responses::GetPlayersResponse{.players = players}))
                .no_cache();
        }
    };
    static constexpr std::string_view endpoint{"/api/v1/game/players"};
};