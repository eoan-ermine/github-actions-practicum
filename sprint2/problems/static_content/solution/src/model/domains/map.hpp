#pragma once

#include <boost/json.hpp>
#include <unordered_map>

#include "basic.hpp"
#include "util/tagged.hpp"

namespace model {

using namespace boost::json;

class Road {
  public:
    Road(Orientation orientation, Point start, Coord end_dimension) noexcept
        : orientation_(orientation), start_{start} {
        switch (orientation) {

        case Orientation::HORIZONTAL:
            end_ = {end_dimension, start.y};
            break;
        case Orientation::VERTICAL:
            end_ = {start.x, end_dimension};
            break;
        }
    }

    Orientation GetOrientation() const noexcept { return orientation_; }

    bool IsHorizontal() const noexcept { return orientation_ == Orientation::HORIZONTAL; }

    bool IsVertical() const noexcept { return orientation_ == Orientation::VERTICAL; }

    Point GetStart() const noexcept { return start_; }

    Point GetEnd() const noexcept { return end_; }

  private:
    Orientation orientation_;
    Point start_;
    Point end_;
};

// Serialize road structure to json value
void tag_invoke(value_from_tag, value &value, const Road &road);
// Deserialize json value to road structure
Road tag_invoke(value_to_tag<Road>, const value &value);

class Building {
  public:
    explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

    const Rectangle &GetBounds() const noexcept { return bounds_; }

  private:
    Rectangle bounds_;
};

// Serialize building structure to json value
void tag_invoke(value_from_tag, value &value, const Building &building);
// Deserialize json value to building structure
Building tag_invoke(value_to_tag<Building>, const value &value);

class Office {
  public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept : id_{std::move(id)}, position_{position}, offset_{offset} {}

    const Id &GetId() const noexcept { return id_; }

    Point GetPosition() const noexcept { return position_; }

    Offset GetOffset() const noexcept { return offset_; }

  private:
    Id id_;
    Point position_;
    Offset offset_;
};

// Serialize office structure to json value
void tag_invoke(value_from_tag, value &value, const Office &office);
// Deserialize json value to office structure
Office tag_invoke(value_to_tag<Office>, const value &value);

class Map {
  public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using PointsToRoads =
        std::unordered_map<Orientation, std::unordered_map<Point, const Road *, decltype([](const Point &point) {
                                                               return std::hash<int>()(point.x) ^
                                                                      std::hash<int>()(point.y);
                                                           })>>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

    Map(Id id, std::string name, Roads &&roads, Buildings &&buildings, Offices &&offices) noexcept
        : Map(std::move(id), std::move(name)) {
        roads_ = std::move(roads);
        buildings_ = std::move(buildings);
        for (auto &&office : offices) {
            AddOffice(std::move(office));
        }
        for (const auto &road : roads) {
            auto current_point = road.GetStart(), end_point = road.GetEnd();
            if (road.IsHorizontal()) {
                while (current_point.x <= end_point.x) {
                    point_to_road_[Orientation::HORIZONTAL][current_point] = &road;
                    current_point.x += 1;
                }
            } else if (road.IsVertical()) {
                while (current_point.y <= end_point.y) {
                    point_to_road_[Orientation::VERTICAL][current_point] = &road;
                    current_point.y += 1;
                }
            }
        }
    }

    const Id &GetId() const noexcept { return id_; }

    const std::string &GetName() const noexcept { return name_; }

    const Buildings &GetBuildings() const noexcept { return buildings_; }

    const Roads &GetRoads() const noexcept { return roads_; }

    const PointsToRoads &GetPointsToRoads() const noexcept { return point_to_road_; }

    const Offices &GetOffices() const noexcept { return offices_; }

    void SetDogSpeed(double dog_speed) { dog_speed_ = dog_speed; }

    std::optional<double> GetDogSpeed() { return dog_speed_; }

  private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t>;

    void AddOffice(Office &&office);

    Id id_;
    std::string name_;
    Roads roads_;
    PointsToRoads point_to_road_;
    Buildings buildings_;
    std::optional<double> dog_speed_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

// Serialize map structure to json value
void tag_invoke(value_from_tag, value &value, const Map &map);
// Deserialize json value to map structure
Map tag_invoke(value_to_tag<Map>, const value &value);

} // namespace model