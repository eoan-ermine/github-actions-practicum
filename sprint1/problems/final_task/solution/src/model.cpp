#include "model.hpp"

#include <stdexcept>

namespace model {

using namespace std::literals;

std::string to_string(boost::json::string string) { return {string.begin(), string.end()}; }

void tag_invoke(value_from_tag, value &value, const Road &road) {
    const auto [start_x, start_y] = road.GetStart();
    const auto [end_x, end_y] = road.GetEnd();
    bool is_vertical = road.IsVertical();

    value = {{"x0"sv, start_x}, {"y0"sv, start_y}, {is_vertical ? "y1"sv : "x1"sv, is_vertical ? end_y : end_x}};
}

Road tag_invoke(value_to_tag<Road>, const value &value) {
    const object &obj = value.as_object();

    int start_x = obj.at("x0"sv).as_int64();
    int start_y = obj.at("y0"sv).as_int64();

    if (obj.count("x1")) {
        int end_x = obj.at("x1"sv).as_int64();
        return Road{Road::HORIZONTAL, Point{start_x, start_y}, end_x};
    } else {
        int end_y = obj.at("y1"sv).as_int64();
        return Road{Road::VERTICAL, Point{start_x, start_y}, end_y};
    }
}

void tag_invoke(value_from_tag, value &value, const Building &building) {
    const auto [position, size] = building.GetBounds();

    value = {{"x"sv, position.x}, {"y"sv, position.y}, {"w"sv, size.width}, {"h"sv, size.height}};
}

Building tag_invoke(value_to_tag<Building>, const value &value) {
    const object &obj = value.as_object();

    int x = obj.at("x"sv).as_int64();
    int y = obj.at("y"sv).as_int64();
    int width = obj.at("w"sv).as_int64();
    int height = obj.at("h"sv).as_int64();

    return Building{Rectangle{Point{x, y}, Size{width, height}}};
}

void tag_invoke(value_from_tag, value &value, const Office &office) {
    const auto &id = office.GetId();
    const auto &position = office.GetPosition();
    const auto &offset = office.GetOffset();

    value = {
        {"id"sv, *id}, {"x"sv, position.x}, {"y"sv, position.y}, {"offsetX"sv, offset.dx}, {"offsetY"sv, offset.dy}};
}

Office tag_invoke(value_to_tag<Office>, const value &value) {
    const object &obj = value.as_object();

    auto id = to_string(obj.at("id"sv).as_string());
    int x = obj.at("x"sv).as_int64();
    int y = obj.at("y"sv).as_int64();
    int x_offset = obj.at("offsetX"sv).as_int64();
    int y_offset = obj.at("offsetY"sv).as_int64();

    return Office{Office::Id(id), Point{x, y}, Offset{x_offset, y_offset}};
}

void tag_invoke(value_from_tag, value &value, const Map &map) {
    value = {{"id", *map.GetId()},
             {"name", map.GetName()},
             {"roads", value_from(map.GetRoads())},
             {"buildings", value_from(map.GetBuildings())},
             {"offices", value_from(map.GetOffices())}};
}

Map tag_invoke(value_to_tag<Map>, const value &value) {
    const object &obj = value.as_object();

    auto id = to_string(obj.at("id"sv).as_string());
    auto name = to_string(obj.at("name"sv).as_string());

    return {Map::Id{id}, name, value_to<std::vector<Road>>(obj.at("roads")),
            value_to<std::vector<Building>>(obj.at("buildings")), value_to<std::vector<Office>>(obj.at("offices"))};
}

Game tag_invoke(value_to_tag<Game>, const value &value) {
    const object &obj = value.as_object();

    return Game{value_to<std::vector<Map>>(obj.at("maps"))};
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

void Map::AddOffice(Office &&office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office &o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
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
