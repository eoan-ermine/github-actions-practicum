#include "filesystem.hpp"

namespace fs = std::filesystem;

namespace {

// Возвращает true, если каталог path содержится внутри base.
bool IsSubPath(fs::path path, fs::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}

} // namespace

namespace util {

fs::path GetPath(std::string_view target, fs::path base_path) {
    using namespace std::literals;

    // Remove slash symbol prefix
    target.remove_prefix(1);

    fs::path rel_path(target);
    if (target.empty()) {
        rel_path.append("index.html"sv);
    }
    return fs::weakly_canonical(base_path / rel_path);
}

bool ValidatePath(fs::path path, fs::path base_path) { return IsSubPath(path, base_path); }

} // namespace util
