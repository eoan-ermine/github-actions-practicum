#include "game.hpp"

using namespace std::literals;

namespace model {

Game tag_invoke(value_to_tag<Game>, const value &value) {
    const object &obj = value.as_object();
    auto maps = value_to<std::vector<Map>>(obj.at("maps"));

    double default_dog_speed = 1.0;
    if (obj.contains("defaultDogSpeed")) {
        default_dog_speed = obj.at("defaultDogSpeed").as_double();
    }
    for (auto &map : maps) {
        if (!map.GetDogSpeed())
            map.SetDogSpeed(default_dog_speed);
    }

    return Game{std::move(maps)};
}

void tag_invoke(value_from_tag, value &value, const Game::Maps &maps) {
    array maps_array;

    for (const auto &map : maps) {
        object res_object;
        res_object["id"sv] = *map.GetId();
        res_object["name"sv] = map.GetName();
        maps_array.push_back(res_object);
    }

    value = maps_array;
}

void Game::AddMap(Map &&map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

} // namespace model