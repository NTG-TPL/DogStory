#include "dog.h"

namespace model {

Dog::Id Dog::GetId() const noexcept {
    return id_;
}

const std::string& Dog::GetName() const noexcept {
    return name_;
}

void Dog::Move(std::string_view direction, DimensionDouble speed) {
    using namespace std::string_literals;
    auto it = Movement::MOVEMENT_VIEW.find(direction);
    direction_ = (direction != Movement::STOP) ? *it : direction_;
    speed_ = Movement::MOVEMENT.at(direction)(speed);
}

void Dog::SetPosition(Point2d new_point){
    position_ = new_point;
}

void Dog::Stand(){
    speed_ = Movement::Stand();
}

void Dog::PutToBag(const FoundObject& loot) {
    bag_.push_back(loot);
}

void Dog::SetSpeed(Velocity2d speed) noexcept {
    speed_ = speed;
}

void Dog::SetDirection(std::string_view direction) {
    auto it = Movement::MOVEMENT_VIEW.find(direction);
    direction_ = (direction != Movement::STOP) ? *it : direction_;
}

void Dog::AddScore(std::int32_t score) noexcept {
    score_ += score;
}

Velocity2d Dog::GetSpeed() const noexcept {
    return speed_;
}

std::string_view Dog::GetDirection() const noexcept {
    return direction_;
}

const Dog::Bag& Dog::GetBag() const noexcept {
    return bag_;
}

[[nodiscard]] std::int32_t Dog::GetScore() const noexcept {
    return score_;
}

void Dog::BagClear() noexcept {
    score_ += std::accumulate(bag_.begin(), bag_.end(), 0, [](std::int32_t lhs, FoundObject& el){
        return lhs + el.value;
    });
    bag_.clear();
}

void Dog::UpdateLifeTimer(std::chrono::milliseconds delta_time) {
    using namespace std::literals::chrono_literals;
    life_time_ += delta_time;
    stay_time_ = (speed_ == Movement::Stand()) ? (stay_time_ += delta_time) : 0ms;
}

std::chrono::milliseconds Dog::GetStayTime() const {
    return stay_time_;
}

std::chrono::milliseconds Dog::GetLifeTime() const {
    return life_time_;
}

} // namespace model