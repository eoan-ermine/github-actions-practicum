#include <filesystem>
#include <string_view>

namespace util {

namespace fs = std::filesystem;

fs::path GetPath(std::string_view target, fs::path base_path);
bool ValidatePath(fs::path path, fs::path base_path);

} // namespace util