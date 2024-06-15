#include "game.h"

namespace model {
using namespace std::string_literals;

void Game::AddMap(const Map& map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = id_to_map_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::make_shared<Map>(map));
        } catch (...) {
            id_to_map_index_.erase(it);
            throw;
        }
    }
}

const Game::Maps& Game::GetMaps() const noexcept {
    return maps_;
}

std::shared_ptr<const Map> Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = id_to_map_index_.find(id); it != id_to_map_index_.end()) {
        return maps_.at(it->second);
    }
    return nullptr;
}

std::shared_ptr<GameSession> Game::UpdateSessionFullness(GameSession::Id index, const GameSession& session) {
    auto id_session = id_to_session.at(index);
    map_to_sessions_[session.GetMapId()].emplace(session.AmountAvailableSeats(), id_session);
    return sessions_.at(id_session);
}

std::pair<GameSession::Id, std::shared_ptr<GameSession>> Game::CreateFreeSession(const Map::Id& map_id) {
    auto map = FindMap(map_id);
    if (map == nullptr) {
        throw std::invalid_argument("Map id \""s + *map_id + "\" does not exist"s);
    }
    auto ms = std::chrono::milliseconds(static_cast<size_t>(period_*1000));
    auto& session = sessions_.emplace_back(std::make_shared<GameSession>(GameSession::Id{game_session_id_++},
                                                                         map,
                                                                         loot_gen::LootGenerator(ms, probability_)));
    id_to_session.emplace(session->GetId(), sessions_.size() - 1);
    return {session->GetId(), session};
}

std::optional<std::pair<GameSession::Id, std::shared_ptr<GameSession>>> Game::ExtractFreeSession(const Map::Id& map_id) {
    if (map_to_sessions_.contains(map_id) &&
        !map_to_sessions_.at(map_id).empty() &&
        map_to_sessions_.at(map_id).begin()->first > 0) { // количество свободных мест > 0;
        auto& sessions = map_to_sessions_.at(map_id);
        auto index = GameSession::Id{sessions.begin()->second};
        if(sessions_.at(*index) == nullptr){ // создадим новую сессию и поменяем указатель на неё.
            return std::pair<GameSession::Id, std::shared_ptr<GameSession>>{index, nullptr};
        }
        sessions.extract(sessions.begin()->first);
        return std::pair<GameSession::Id, std::shared_ptr<GameSession>>{index, sessions_.at(*index)}; // нужная сессия уже существует
    }
    return std::nullopt; // создадим новую сессию
}

void Game::Update(std::chrono::milliseconds tick){
    for (auto& el: sessions_) {
        if(el != nullptr){
            el->Update(tick);
        }
    }
}

void Game::SetLootGeneratorConfig(double period, double probability) noexcept {
    using namespace std::chrono_literals;
    period_ = period;
    probability_ = probability;
}

double Game::GetLootPeriod() const noexcept{
    return period_;
}

double Game::GetLootProbability() const noexcept{
    return probability_;
}

const Game::Sessions& Game::GetSessions() const noexcept {
    return sessions_;
}

    std::chrono::milliseconds Game::GetDogRetirementTime() const noexcept {
    return retirement_time_;
}

void Game::SetDogRetirementTime(std::chrono::milliseconds retirement_time) noexcept {
    retirement_time_ = retirement_time;
}

std::shared_ptr<GameSession> Game::FindSession(GameSession::Id id) const noexcept {
    return id_to_session.contains(id) ? sessions_.at(id_to_session.at(id)) : nullptr;
}

void Game::AddSession(const GameSession& session) {
    sessions_.push_back(std::make_shared<GameSession>(session));
    // Меняет счётчик id, чтобы не допускать пересечения индексов в дальнейшем при создании
    game_session_id_ = (*session.GetId() >= game_session_id_ ||
                        sessions_.size() > game_session_id_) ?
                                std::max(*session.GetId() + 1, sessions_.size()):
                                game_session_id_;

    auto id_session = sessions_.size() - 1;
    id_to_session[session.GetId()] = id_session;
    map_to_sessions_[session.GetMapId()].emplace(session.AmountAvailableSeats(), id_session);
}

void Game::AddFreeSession(const Map::Id& id) {
    auto map = FindMap(id);
    sessions_.push_back(nullptr);
    // Меняет счётчик id, чтобы не допускать пересечения индексов в дальнейшем при создании
    game_session_id_ = (sessions_.size() > game_session_id_) ? sessions_.size(): game_session_id_;
    auto id_session = sessions_.size() - 1;
    id_to_session[GameSession::Id{game_session_id_}] = id_session;
    map_to_sessions_[id].emplace(map->GetLimitPlayers(), id_session);
}

} // namespace model