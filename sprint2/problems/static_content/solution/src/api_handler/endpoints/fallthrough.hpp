#pragma once

#include "api_handler/endpoints/endpoint.hpp"

class FallthroughEndpoint : public Endpoint {
  public:
    using Endpoint::Endpoint;
    bool match(const http::request<http::string_body> &request) override {
        return request.target().starts_with("/api/");
    }
    util::Response handle(const http::request<http::string_body> &request) override {
        return model::api::errors::invalid_endpoint();
    }
};