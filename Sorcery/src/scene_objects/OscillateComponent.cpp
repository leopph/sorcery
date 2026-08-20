#include "OscillateComponent.h"

#include "Entity.hpp"
#include "../gui_helpers.hpp"
#include "../Timing.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::OscillateComponent>("Oscillate Component")
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("speed", &sorcery::OscillateComponent::speed_)
    .property("direction", &sorcery::OscillateComponent::direction_)
    .property("distance", &sorcery::OscillateComponent::distance_);
}


namespace sorcery {
auto OscillateComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<OscillateComponent>(*this);
}


auto OscillateComponent::Start() -> void {
  Component::Start();
  cur_dist_ = 0;
  going_backward_ = false;
}


auto OscillateComponent::Update() -> void {
  auto& transform{GetEntity()->GetTransform()};

  auto const this_frame_progress{speed_ * timing::GetFrameTime()};

  if (going_backward_) {
    cur_dist_ -= this_frame_progress;
  } else {
    cur_dist_ += this_frame_progress;
  }

  transform.Translate(direction_ * this_frame_progress * (going_backward_ ? -1.f : 1.f), Space::Local);

  if (std::abs(cur_dist_) >= distance_) {
    going_backward_ = !going_backward_;
  }
}


OscillateComponent::OscillateComponent() {
  SetUpdatable(true);
}


auto OscillateComponent::GetDirection() const -> Vector3 const& {
  return direction_;
}


auto OscillateComponent::SetDirection(Vector3 const& dir) -> void {
  direction_ = dir;
}


auto OscillateComponent::GetSpeed() const -> float {
  return speed_;
}


auto OscillateComponent::SetSpeed(float const speed) -> void {
  speed_ = speed;
}


auto OscillateComponent::GetDistance() const -> float {
  return distance_;
}


auto OscillateComponent::SetDistance(float const dist) -> void {
  distance_ = dist;
}
}
