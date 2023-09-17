#include "basic.hpp"
#include <utility>

using namespace std::literals;

namespace model {

Direction parse(std::string_view direction) {
    if (direction.empty())
        return Direction::NO;

    switch (direction[0]) {
    case 'L':
        return Direction::WEST;
    case 'R':
        return Direction::EAST;
    case 'U':
        return Direction::NORTH;
    case 'D':
        return Direction::SOUTH;
    }
}

std::string_view serialize(Direction direction) {
    switch (direction) {
    case model::Direction::EAST:
        return "E";
    case model::Direction::NORTH:
        return "N";
    case model::Direction::SOUTH:
        return "S";
    case model::Direction::WEST:
        return "W";
    }
}

} // namespace model