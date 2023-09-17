#include "response.hpp"

namespace util {

Response Response::Text(http::status status, std::string_view body) {
    StringResponse response;
    response.result(status);
    response.set(http::field::content_type, "text/plain");
    response.body() = body;
    response.content_length(body.size());

    Response result;
    result = std::move(response);
    return result;
}

Response Response::Json(http::status status, const json::value &value) {
    StringResponse response;
    response.result(status);
    response.set(http::field::content_type, "application/json");
    response.body() = json::serialize(value);
    response.content_length(response.body().size());

    Response result;
    result = std::move(response);
    return result;
}

Response Response::File(http::status status, std::string_view mime_type, std::string_view filepath,
                        boost::system::error_code &ec) {
    FileResponse response;
    response.result(status);
    response.set(http::field::content_type, mime_type);

    http::file_body::value_type file;
    file.open(filepath.data(), beast::file_mode::read, ec);
    if (!ec) {
        response.body() = std::move(file);
        response.prepare_payload();
    }

    Response result;
    result = std::move(response);
    return result;
}

Response &Response::operator=(StringResponse &&response_) {
    response = std::move(response_);
    return *this;
}
Response &Response::operator=(FileResponse &&response_) {
    response = std::move(response_);
    return *this;
}

int Response::code() const {
    int code;
    std::visit([&](auto &&arg) { code = arg.result_int(); }, response);
    return code;
}

std::string_view Response::content_type() const {
    std::string_view content_type;
    std::visit(
        [&](auto &&arg) {
            content_type = arg.count(http::field::content_type) ? arg[http::field::content_type] : "null";
        },
        response);
    return content_type;
}

void Response::set(std::string_view name, std::string_view value) {
    std::visit([&](auto &&arg) { arg.set(name, value); }, response);
}

Response &&Response::no_cache() && {
    set("Cache-Control", "no-cache");
    return std::move(*this);
};

Response &&Response::allow(std::string_view allowed_methods) && {
    set("Allow", allowed_methods);
    return std::move(*this);
}

void Response::finalize(unsigned http_version, bool keep_alive) {
    std::visit([&](auto &&arg) { FinalizeResponse(arg, http_version, keep_alive); }, response);
}

} // namespace util