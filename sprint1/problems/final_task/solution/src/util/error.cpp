#include "error.hpp"

#include <boost/json.hpp>

namespace util {

using namespace std::literals;

void tag_invoke(value_from_tag, value &value, const Error &error) {
    value = {
        {"code"sv, error.code},
        {"message"sv, error.message},
    };
}

} // namespace util