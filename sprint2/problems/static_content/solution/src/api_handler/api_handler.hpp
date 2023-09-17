#pragma once

#include "model/model.hpp"
#include "util/response.hpp"

#include "endpoints/endpoints.hpp"

namespace api_handler {

using namespace util;

using verb = boost::beast::http::verb;

class APIHandler {
  public:
    APIHandler(model::Game &game) : game_(game), endpoints_(GetEndpoints(game_)) {}

    template <typename Body, typename Allocator>
    bool dispatch(const http::request<Body, http::basic_fields<Allocator>> &request, Response &response) const {
        for (const auto &endpoint : endpoints_) {
            if (endpoint->match(request)) {
                response = endpoint->handle(request);
                return true;
            }
        }

        return false;
    }

  private:
    model::Game &game_;
    std::vector<std::shared_ptr<Endpoint>> endpoints_;
};

} // namespace api_handler
