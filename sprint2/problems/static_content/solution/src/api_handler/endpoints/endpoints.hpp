#include "endpoint.hpp"

#include "fallthrough.hpp"
#include "game/join.hpp"
// #include "game/player/action.hpp"
#include "game/player/get_players.hpp"
// #include "game/state/get_state.hpp"
// #include "game/tick.hpp"
#include "map/get_map.hpp"
#include "map/get_maps.hpp"
#include <memory>

inline std::vector<std::shared_ptr<Endpoint>> GetEndpoints(model::Game &game) {
    return {std::shared_ptr<Endpoint>{new GetMapEndpoint{game}}, std::shared_ptr<Endpoint>{new GetMapsEndpoint{game}},
            std::shared_ptr<Endpoint>{new JoinEndpoint{game}}, std::shared_ptr<Endpoint>{new GetPlayersEndpoint(game)},
            // std::shared_ptr<Endpoint>{new GetStateEndpoint(game)},
            // std::shared_ptr<Endpoint>{new ActionEndpoint{game}},
            // std::shared_ptr<Endpoint>{new TickEndpoint(game)},
            std::shared_ptr<Endpoint>{new FallthroughEndpoint(game)}};
}