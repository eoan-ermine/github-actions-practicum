#include <string_view>

namespace util {

// Return a reasonable mime type based on the extension of a file.
std::string_view GetMimeType(std::string_view path);

} // namespace util