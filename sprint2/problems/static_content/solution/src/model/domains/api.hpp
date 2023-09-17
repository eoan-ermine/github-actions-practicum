#pragma once

#include <boost/json.hpp>

#include <string>

#include "basic.hpp"
#include "game.hpp"
#include "map.hpp"
#include "util/error.hpp"
#include "util/response.hpp"

namespace model {

using namespace boost::json;

namespace api::requests {

struct JoinRequest {
    std::string userName;
    Map::Id::ValueType mapId;
};

// Deserialize json value to join request
JoinRequest tag_invoke(value_to_tag<JoinRequest>, const value &value);

struct ActionRequest {
    Direction move;
};

// Deserialize json value to action request
ActionRequest tag_invoke(value_to_tag<ActionRequest>, const value &value);

struct TickRequest {
    std::chrono::milliseconds timedelta;
};

// Deserialize json value to tick request
TickRequest tag_invoke(value_to_tag<TickRequest>, const value &value);

} // namespace api::requests

namespace api::responses {

struct JoinResponse {
    Token authToken;
    Player::Id playerId;
};

// Serialize join response to json value
void tag_invoke(value_from_tag, value &value, const JoinResponse &response);

struct GetPlayersResponse {
    const std::unordered_map<Player::Id, std::shared_ptr<Player>> &players;
};

// Serialize get players response to json value
void tag_invoke(value_from_tag, value &value, const GetPlayersResponse &response);

struct GetStateResponse {
    const std::unordered_map<Player::Id, std::shared_ptr<Player>> &players;
};

// Serialize get state response to json value
void tag_invoke(value_from_tag, value &value, const GetStateResponse &response);

} // namespace api::responses

namespace api::errors {

using boost::beast::http::status;

static util::Response no_token() {
    return util::Response::Json(
               status::unauthorized,
               value_from(util::Error{.code = "invalidToken", .message = "Authorization header is missing"}))
        .no_cache();
}

static util::Response only_get_and_head() {
    return util::Response::Json(
               status::method_not_allowed,
               value_from(util::Error{.code = "invalidMethod", .message = "Only GET and HEAD methods are expected"}))
        .no_cache()
        .allow("GET, HEAD");
}

static util::Response no_user_found() {
    auto response = util::Response::Json(
                        status::unauthorized,
                        value_from(util::Error{.code = "invalidToken", .message = "Player token has not been found"}))
                        .no_cache();
    return response;
}

static util::Response only_post() {
    return util::Response::Json(
               status::method_not_allowed,
               value_from(util::Error{.code = "invalidMethod", .message = "Only POST method is expected"}))
        .no_cache()
        .allow("POST");
}

static util::Response parse_error() {
    return util::Response::Json(status::bad_request,
                                value_from(util::Error{.code = "invalidArgument", .message = "Parse error"}))
        .no_cache();
}

static util::Response invalid_username() {
    return util::Response::Json(status::bad_request,
                                value_from(util::Error{.code = "invalidArgument", .message = "Invalid username"}))
        .no_cache();
}

static util::Response map_not_found() {
    return util::Response::Json(status::not_found,
                                value_from(util::Error{.code = "mapNotFound", .message = "Map not found"}))
        .no_cache();
}

static util::Response invalid_content_type() {
    return util::Response::Json(status::bad_request,
                                value_from(util::Error{.code = "invalidArgument", .message = "Invalid content type"}))
        .no_cache();
}

static util::Response invalid_endpoint() {
    return util::Response::Json(status::bad_request,
                                value_from(util::Error{.code = "badRequest", .message = "Invalid endpoint"}))
        .no_cache();
}

} // namespace api::errors

} // namespace model