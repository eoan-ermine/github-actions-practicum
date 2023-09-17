#pragma once

#include <boost/json/conversion.hpp>
#include <boost/json/value.hpp>

namespace util {

using namespace boost::json;

struct Error {
    std::string_view code;
    std::string_view message;
};

void tag_invoke(value_from_tag, value &value, const Error &error);

} // namespace util