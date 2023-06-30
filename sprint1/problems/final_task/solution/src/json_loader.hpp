#pragma once

#include <filesystem>

#include "model.hpp"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path &json_path);

} // namespace json_loader
