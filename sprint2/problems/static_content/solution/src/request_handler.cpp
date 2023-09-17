#include "request_handler.hpp"

#include "util/filesystem.hpp"
#include "util/mime_type.hpp"

namespace request_handler {

// Handle static files requests
Response RequestHandler::get_file(std::string_view target) const {
    Response response;

    auto path = GetPath(target, base_path_);
    if (ValidatePath(path, base_path_)) {
        auto extension = path.extension().string();

        boost::system::error_code ec;
        response = Response::File(http::status::ok, GetMimeType(extension), path.string(), ec);
        if (ec)
            response = Response::Text(http::status::not_found, "File not found");
    } else {
        response = Response::Text(http::status::bad_request, "Invalid path");
    }

    return response;
}

} // namespace request_handler