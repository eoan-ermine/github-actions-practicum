#include "json_loader.hpp"

#include <fstream>
#include <iostream>

#include <boost/json.hpp>

namespace json_loader {

using namespace std::literals;
using namespace model;

std::string to_string(boost::json::string string) { return {string.begin(), string.end()}; }

void LoadRoads(Map &map, const boost::json::array &roads) {
    for (const auto &road_element : roads) {
        const auto &road_obj = road_element.as_object();

        int start_x = road_obj.at("x0"sv).as_int64();
        int start_y = road_obj.at("y0"sv).as_int64();

        if (road_obj.count("x1")) {
            int end_x = road_obj.at("x1"sv).as_int64();
            map.AddRoad(Road{Road::HORIZONTAL, Point{start_x, start_y}, end_x});
        } else {
            int end_y = road_obj.at("y1"sv).as_int64();
            map.AddRoad(Road{Road::VERTICAL, Point{start_x, start_y}, end_y});
        }
    }
}

void LoadBuildings(Map &map, const boost::json::array &buildings) {
    for (const auto &building_element : buildings) {
        const auto &building_obj = building_element.as_object();

        int x = building_obj.at("x"sv).as_int64();
        int y = building_obj.at("y"sv).as_int64();
        int width = building_obj.at("w"sv).as_int64();
        int height = building_obj.at("h"sv).as_int64();

        map.AddBuilding(Building{Rectangle{Point{x, y}, Size{width, height}}});
    }
}

void LoadOffices(Map &map, const boost::json::array &offices) {
    for (const auto &office_element : offices) {
        const auto &office_obj = office_element.as_object();

        auto id = to_string(office_obj.at("id"sv).as_string());
        int x = office_obj.at("x"sv).as_int64();
        int y = office_obj.at("y"sv).as_int64();
        int x_offset = office_obj.at("offsetX"sv).as_int64();
        int y_offset = office_obj.at("offsetY"sv).as_int64();

        map.AddOffice(Office{Office::Id(id), Point{x, y}, Offset{x_offset, y_offset}});
    }
}

model::Game LoadGame(const std::filesystem::path &json_path) {
    Game game;

    std::ifstream stream(json_path);
    std::stringstream buffer;
    buffer << stream.rdbuf();

    auto obj = boost::json::parse(buffer.str()).as_object();

    const auto &maps = obj.at("maps").as_array();
    for (const auto &map_element : maps) {
        const auto &map_obj = map_element.as_object();

        auto id = to_string(map_obj.at("id"sv).as_string());
        auto name = to_string(map_obj.at("name"sv).as_string());
        Map map(Map::Id{id}, name);

        LoadRoads(map, map_obj.at("roads"sv).as_array());
        LoadBuildings(map, map_obj.at("buildings"sv).as_array());
        LoadOffices(map, map_obj.at("offices"sv).as_array());

        game.AddMap(std::move(map));
    }

    return game;
}

} // namespace json_loader
