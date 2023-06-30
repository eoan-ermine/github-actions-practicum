#pragma once

#include <boost/json.hpp>

#include "model.hpp"

namespace json_serializer {

using namespace model;

boost::json::value SerializeError(std::string_view code, std::string_view message);
boost::json::value Serialize(const Game::Maps &maps);
boost::json::value Serialize(const Map &map);

} // namespace json_serializer
