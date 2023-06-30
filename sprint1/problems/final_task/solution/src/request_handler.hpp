#pragma once

#include <boost/json.hpp>

#include "http_server.hpp"
#include "model.hpp"
#include "util/error.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive,
                                  std::string_view content_type = "text/html"sv);

class RequestHandler {
  public:
    explicit RequestHandler(model::Game &game) : game_{game} {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>> &&request, Send &&send) {
        auto json_response = [&request](http::status status, json::value value) {
            return MakeStringResponse(status, json::serialize(value), request.version(), request.keep_alive(),
                                      "application/json"sv);
        };

        auto target = request.target();
        std::string_view endpoint = "/api/v1/maps";

        if (target == endpoint) {
            send(json_response(http::status::ok, json::value_from(game_.GetMaps())));
        } else if (target.starts_with(endpoint) && !target.ends_with("/"sv)) {
            std::string_view id = target.substr(endpoint.size() + 1);
            auto map_id = model::Map::Id{std::string{id}};
            const auto *map_ptr = game_.FindMap(map_id);

            if (map_ptr == nullptr)
                send(json_response(http::status::not_found,
                                   json::value_from(util::Error{"mapNotFound"sv, "Map not found"sv})));
            else
                send(json_response(http::status::ok, json::value_from(*map_ptr)));
        } else if (target.starts_with("/api/"sv)) {
            send(json_response(http::status::bad_request,
                               json::value_from(util::Error{"badRequest"sv, "Bad request"sv})));
        }
    }

  private:
    model::Game &game_;
};

} // namespace request_handler
