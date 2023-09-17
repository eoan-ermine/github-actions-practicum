#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <chrono>
#include <filesystem>

#include "api_handler/api_handler.hpp"
#include "model/model.hpp"
#include "util/logging.hpp"
#include "util/response.hpp"

namespace request_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;

using namespace std::literals;
using namespace util;

using StringRequest = http::request<http::string_body>;

class RequestHandler {
  public:
    using Strand = beast::net::strand<beast::net::io_context::executor_type>;

    explicit RequestHandler(model::Game &game, std::string_view static_path, Strand api_strand)
        : api_(game), base_path_(static_path), api_strand_(api_strand) {}

    RequestHandler(const RequestHandler &) = delete;
    RequestHandler &operator=(const RequestHandler &) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(std::string_view address, http::request<Body, http::basic_fields<Allocator>> &&request,
                    Send &&send) const {
        auto target = request.target();

        LogRequest(address, target, request.method_string());
        auto start_ts = std::chrono::system_clock::now();

        Response response;
        bool is_api_request = api_.dispatch(request, response);
        if (!is_api_request) {
            response = get_file(target);
        }

        auto end_ts = std::chrono::system_clock::now();
        LogResponse(std::chrono::duration_cast<std::chrono::milliseconds>(end_ts - start_ts).count(), response.code(),
                    response.content_type());

        response.finalize(request.version(), request.keep_alive());
        if (is_api_request)
            response.send(send, api_strand_);
        else
            response.send(send);
    }

  private:
    // Handle static files requests
    Response get_file(std::string_view target) const;

    api_handler::APIHandler api_;
    fs::path base_path_;
    Strand api_strand_;
};

} // namespace request_handler
