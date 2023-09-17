#pragma once

#include <boost/asio/strand.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>

#include <memory>
#include <string_view>
#include <variant>

namespace util {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

class Response : public std::enable_shared_from_this<Response> {
  public:
    using Strand = beast::net::strand<beast::net::io_context::executor_type>;

    Response() {}

    static Response Text(http::status status, std::string_view body);
    static Response Json(http::status status, const json::value &value);
    static Response File(http::status status, std::string_view mime_type, std::string_view filepath,
                         boost::system::error_code &ec);

    Response &operator=(StringResponse &&response_);
    Response &operator=(FileResponse &&response_);

    int code() const;
    std::string_view content_type() const;
    void set(std::string_view name, std::string_view value);
    Response &&no_cache() &&;
    Response &&allow(std::string_view allowed_methods) &&;

    void finalize(unsigned http_version, bool keep_alive);

    template <typename Send>
    void send(Send &&send_fn) {
        std::visit([send_fn = std::move(send_fn)](auto &&arg) { send_fn(std::move(arg)); }, response);
    }

    template <typename Send>
    void send(Send &&send_fn, Strand api_strand) {
        boost::beast::net::dispatch(
            api_strand, [send_fn = std::move(send_fn), response = std::move(this->response)]() mutable {
                std::visit([send_fn = std::move(send_fn)](auto &&arg) { send_fn(std::move(arg)); }, response);
            });
    }

  private:
    template <typename Body>
    static void FinalizeResponse(http::response<Body> &response, unsigned http_version, bool keep_alive) {
        response.version(http_version);
        response.keep_alive(keep_alive);
    }

    std::variant<StringResponse, FileResponse> response;
};

} // namespace util