#pragma once

#include <filesystem>

#include "model/model.hpp"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path &json_path);

} // namespace json_loader
