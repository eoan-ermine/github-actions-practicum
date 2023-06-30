#include "json_loader.hpp"

#include <fstream>
#include <iostream>

#include <boost/json.hpp>

namespace json_loader {

model::Game LoadGame(const std::filesystem::path &json_path) {
    std::ifstream stream(json_path);
    std::stringstream buffer;
    buffer << stream.rdbuf();

    return value_to<model::Game>(boost::json::parse(buffer.str()));
}

} // namespace json_loader
