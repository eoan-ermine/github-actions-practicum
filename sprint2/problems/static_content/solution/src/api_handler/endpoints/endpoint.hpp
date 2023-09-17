#pragma once

#include "model/model.hpp"
#include "util/error.hpp"
#include "util/response.hpp"
#include <boost/beast/http.hpp>
#include <boost/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

class Endpoint {
  public:
    Endpoint(model::Game &game) : game_(game) {}
    virtual bool match(const http::request<http::string_body> &request) = 0;
    virtual util::Response handle(const http::request<http::string_body> &request) = 0;

  protected:
    model::Game &game_;
};