#include "mime_type.hpp"

#include <boost/beast/core/string.hpp>

namespace util {

// Return a reasonable mime type based on the extension of a file.
std::string_view GetMimeType(std::string_view path) {
    using boost::beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == std::string_view::npos)
            return std::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))
        return "text/html";
    if (iequals(ext, ".html"))
        return "text/html";
    if (iequals(ext, ".css"))
        return "text/css";
    if (iequals(ext, ".txt"))
        return "text/plain";
    if (iequals(ext, ".js"))
        return "application/javascript";
    if (iequals(ext, ".json"))
        return "application/json";
    if (iequals(ext, ".xml"))
        return "application/xml";
    if (iequals(ext, ".png"))
        return "image/png";
    if (iequals(ext, ".jpg"))
        return "image/jpeg";
    if (iequals(ext, ".jpe"))
        return "image/jpeg";
    if (iequals(ext, ".jpeg"))
        return "image/jpeg";
    if (iequals(ext, ".gif"))
        return "image/gif";
    if (iequals(ext, ".bmp"))
        return "image/bmp";
    if (iequals(ext, ".ico"))
        return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff"))
        return "image/tiff";
    if (iequals(ext, ".tif"))
        return "image/tiff";
    if (iequals(ext, ".svg"))
        return "image/svg+xml";
    if (iequals(ext, ".svgz"))
        return "image/svg+xml";
    return "application/octet-stream";
}

} // namespace util