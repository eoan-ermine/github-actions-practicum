#pragma once

#include <algorithm>
#include <boost/json.hpp>

#include <memory>
#include <random>
#include <sstream>
#include <string>

#include "basic.hpp"
#include "map.hpp"
#include "util/string_hash.hpp"

namespace model {

using namespace boost::json;

// Пес — персонаж, которым управляет игрок.
class Dog {
  public:
    using Id = util::Tagged<std::size_t, Dog>;

    static std::shared_ptr<Dog> Create(std::string name, const Map &map, bool randomize_spawn_points) {
        // Координаты пса — случайно выбранная точка на случайно выбранном отрезке дороги этой карты
        static std::size_t last_id = 0;

        std::pair<double, double> position;
        if (randomize_spawn_points) {
            const auto &roads = map.GetRoads();
            std::mt19937_64 generator{[] {
                std::random_device random_device;
                std::uniform_int_distribution<std::mt19937_64::result_type> dist;
                return dist(random_device);
            }()};

            position = [&]() -> std::pair<double, double> {
                std::uniform_int_distribution<int> uniform_dist(0, roads.size() - 1);
                const auto &road = roads[uniform_dist(generator)];
                auto start = road.GetStart(), end = road.GetEnd();
                if (road.IsVertical()) {
                    std::uniform_int_distribution<int> uniform_dist(start.y, end.y);
                    return {start.x, uniform_dist(generator)};
                } else if (road.IsHorizontal()) {
                    std::uniform_int_distribution<int> uniform_dist(start.x, end.x);
                    return {uniform_dist(generator), start.y};
                }
            }();
        } else {
            auto [x, y] = map.GetRoads()[0].GetStart();
            position = {x, y};
        }

        return std::shared_ptr<Dog>(new Dog(Id{last_id++}, name, position));
    }

    Id GetId() const { return id_; }

    std::string_view GetName() const { return name_; }

    std::pair<double, double> GetPosition() const { return position_; }

    std::pair<double, double> GetSpeed() const { return speed_; }

    Direction GetDirection() const { return direction_; }

    void SetPosition(std::pair<double, double> position) { position_ = position; }

    void SetSpeed(std::pair<double, double> speed) { speed_ = speed; }

    void SetDirection(Direction direction) { direction_ = direction; }

  private:
    // После добавления на карту пёс должен иметь скорость, равную нулю. Направление пса по умолчанию — на север.
    Dog(Id id, std::string name, std::pair<double, double> position)
        : id_(id), name_(std::move(name)), position_(position), speed_({0.0, 0.0}), direction_(Direction::NORTH) {}

    Id id_;
    std::string name_;
    // Координаты пса на карте задаются двумя вещественными числами. Для описания вещественных координат разработайте
    // структуру или класс.
    std::pair<double, double> position_;
    // Скорость пса на карте задаётся также двумя вещественными числами. Скорость измеряется в единицах карты за одну
    // секунду
    std::pair<double, double> speed_;
    // Направление в пространстве принимает одно из четырех значений: NORTH (север), SOUTH (юг), WEST (запад), EAST
    // (восток).
    Direction direction_;
};

// Deserialize json value to dog structure
Dog tag_invoke(value_to_tag<Dog>, const value &value);
// Serialize dog to json value
void tag_invoke(value_from_tag, value &value, const Dog &dog);

class GameSession {
  public:
    using Dogs = std::unordered_map<Dog::Id, std::shared_ptr<Dog>>;

    GameSession(const Map &map) : map_(map) {}

    void AddDog(std::shared_ptr<Dog> dog) { dogs_.insert({dog->GetId(), std::move(dog)}); }

    const Dogs &GetDogs() const { return dogs_; }

    const Map &GetMap() const { return map_; }

  private:
    Dogs dogs_;
    const Map &map_;
};

// Deserialize json value to game session structure
GameSession tag_invoke(value_to_tag<GameSession>, const value &value);
// Serialize game session to json value
void tag_invoke(value_from_tag, value &value, const GameSession &session);

namespace detail {

struct TokenTag {};

} // namespace detail

using Token = util::Tagged<std::string_view, detail::TokenTag>;

class Player {
  public:
    using Id = Dog::Id;

    Player(std::string name, GameSession &session, bool randomize_spawn_points) : session_(session) {
        dog_ = Dog::Create(name, session.GetMap(), randomize_spawn_points);
        session.AddDog(dog_);
    }

    Id GetId() const { return dog_->GetId(); }

    std::string_view GetName() const { return dog_->GetName(); }

    const GameSession &GetSession() const { return session_; }

    std::shared_ptr<Dog> GetDog() const { return dog_; }

  private:
    GameSession &session_;
    std::shared_ptr<Dog> dog_;
};

// Deserialize json value to player structure
Player tag_invoke(value_to_tag<Player>, const value &value);
// Serialize player to json value
void tag_invoke(value_from_tag, value &value, const Player &player);

class PlayerTokens {
  public:
    std::shared_ptr<Player> FindPlayerByToken(std::string_view token) {
        auto it = token_to_player_.find(token);
        if (it == token_to_player_.end()) {
            return nullptr;
        }
        return it->second;
    }

    Token AddPlayer(std::shared_ptr<Player> player) {
        std::stringstream stream;
        stream << std::hex << generator1_() << generator2_();

        std::string token(stream.str());
        auto [it, _] = token_to_player_.insert({token, player});

        return Token{it->first};
    }

  private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

    std::unordered_map<std::string, std::shared_ptr<Player>, string_hash, std::equal_to<>> token_to_player_;
};

class Players {
  public:
    using PlayersContainer = std::unordered_map<Map::Id, std::unordered_map<Dog::Id, std::shared_ptr<Player>>>;

    std::shared_ptr<Player> Add(std::string name, GameSession &session, bool randomize_spawn_points) {
        auto player = std::make_shared<Player>(name, session, randomize_spawn_points);
        const auto &dog = player->GetDog();

        players_[session.GetMap().GetId()][dog->GetId()] = player;
        return player;
    }

    std::shared_ptr<Player> FindByDogIdAndMapId(Dog::Id dog_id, Map::Id map_id) const {
        return players_.at(map_id).at(dog_id);
    }

    const std::unordered_map<Dog::Id, std::shared_ptr<Player>> &GetPlayers(const Map::Id &map_id) {
        return players_[map_id];
    }

    PlayersContainer::iterator begin() { return players_.begin(); }

    PlayersContainer::iterator end() { return players_.end(); }

  private:
    PlayersContainer players_;
};

class Game {
  public:
    using Maps = std::vector<Map>;

    explicit Game(Maps &&maps) {
        for (auto &&map : maps) {
            AddMap(std::move(map));
        }
    }

    const Maps &GetMaps() const noexcept { return maps_; }

    bool ContainsMap(const Map::Id &id) noexcept { return map_id_to_index_.contains(id); }

    Map &GetMap(const Map::Id &id) noexcept { return maps_[map_id_to_index_.at(id)]; }

    void AddSession(GameSession &&session) { sessions_.push_back(std::move(session)); }

    bool ContainsSession(const Map::Id &id) noexcept {
        return std::find_if(sessions_.begin(), sessions_.end(),
                            [&](const auto &session) { return id == session.GetMap().GetId(); }) != sessions_.end();
    }

    GameSession &GetSession(const Map::Id &id) noexcept {
        return *std::find_if(sessions_.begin(), sessions_.end(),
                             [&](const auto &session) { return id == session.GetMap().GetId(); });
    }

    std::pair<std::shared_ptr<Player>, Token> AddPlayer(std::string username, GameSession &session) {
        auto player = players_.Add(std::move(username), session, randomize_spawn_points_);
        auto token = player_tokens_.AddPlayer(player);
        return {player, token};
    }

    std::shared_ptr<Player> GetPlayer(std::string_view token) { return player_tokens_.FindPlayerByToken(token); }

    const std::unordered_map<Dog::Id, std::shared_ptr<Player>> &GetPlayers(const Map::Id &map_id) {
        return players_.GetPlayers(map_id);
    }

    std::optional<int> GetTickPeriod() const { return tick_period_; }

    void SetTickPeriod(int tick_period) { tick_period_ = tick_period; }

    bool GetRandomizeSpawnPoints() const { return randomize_spawn_points_; }

    void SetRandomizeSpawnPoint(bool randomize_spawn_points) { randomize_spawn_points_ = randomize_spawn_points; }

    void Tick(double milliseconds) {
        for (auto &[_, players] : players_) {
            for (auto &[_, player] : players) {
                auto [dx, dy] = player->GetDog()->GetSpeed();
                if (dx == 0 && dy == 0) {
                    continue;
                }

                auto [x, y] = player->GetDog()->GetPosition();
                auto current_point = Point{static_cast<int>(std::round(x)), static_cast<int>(std::round(y))};
                auto [point_x, point_y] = current_point;

                auto map = player->GetSession().GetMap();

                auto road = map.GetPointsToRoads()
                                .at(dx != 0 ? Orientation::HORIZONTAL : Orientation::VERTICAL)
                                .at(current_point);
                auto [start_x, start_y] = road->GetStart();
                auto [end_x, end_y] = road->GetEnd();

                auto need_to_stop = false;
                auto get_new_coordinate = [&](double original_dimension, double dimension_shift, double start_dimension,
                                              double end_dimension) {
                    auto new_coordinate = original_dimension + dimension_shift;
                    if (dimension_shift > 0) {
                        if (new_coordinate > (end_dimension + 0.4)) {
                            need_to_stop = true;
                            return end_dimension + 0.4;
                        }
                    } else if (dimension_shift < 0) {
                        if (new_coordinate < (start_dimension - 0.4)) {
                            need_to_stop = true;
                            return start_dimension - 0.4;
                        }
                    }
                    return new_coordinate;
                };

                if (dx == 0) {
                    y = get_new_coordinate(y, dy * (milliseconds / 1000.0), start_y, end_y);
                } else if (dy == 0) {
                    x = get_new_coordinate(x, dx * (milliseconds / 1000.0), start_x, end_x);
                }
                if (need_to_stop) {
                    player->GetDog()->SetSpeed({0, 0});
                }

                player->GetDog()->SetPosition({x, y});
            }
        }
    }

  private:
    using MapIdToIndex = std::unordered_map<Map::Id, size_t>;

    void AddMap(Map &&map);

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::vector<GameSession> sessions_;
    Players players_;
    PlayerTokens player_tokens_;
    std::optional<int> tick_period_;
    bool randomize_spawn_points_;
};

// Deserialize json value to game structure
Game tag_invoke(value_to_tag<Game>, const value &value);
// Serialize maps to json value
void tag_invoke(value_from_tag, value &value, const Game::Maps &maps);

} // namespace model