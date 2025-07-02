#include "OscillateComponent.h"

#include <imgui.h>

#include "Entity.hpp"
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


auto OscillateComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("Direction");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat3("###OscillateDirection", &direction_[0], 0.01f, -100.0f, 100.0f, "%.2f")) {
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Distance");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat("###OscillateDistance", &distance_, 0.1f, 0.0f, 100.0f, "%.2f")) {
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Speed");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat("###OscillateSpeed", &speed_, 0.1f, 0.0f, 100.0f, "%.2f")) {
    changed = true;
  }
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
}
