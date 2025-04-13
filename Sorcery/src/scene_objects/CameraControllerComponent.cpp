#include "CameraControllerComponent.hpp"

#include <imgui.h>

#include "CameraComponent.hpp"
#include "Entity.hpp"
#include "../app.hpp"
#include "../Platform.hpp"
#include "../Timing.hpp"
#include "../Window.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::CameraControllerComponent>("Camera Controller Component")
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("mouse_sens", &sorcery::CameraControllerComponent::get_mouse_sens,
      &sorcery::CameraControllerComponent::set_mouse_sens)
    .property("move_speed", &sorcery::CameraControllerComponent::get_move_speed,
      &sorcery::CameraControllerComponent::set_move_speed)
    .property("sprint_multiplier", &sorcery::CameraControllerComponent::get_sprint_multiplier,
      &sorcery::CameraControllerComponent::set_sprint_multiplier);
}


namespace sorcery {
auto CameraControllerComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<CameraControllerComponent>(*this);
}


auto CameraControllerComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("Mouse Sensitivity");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat("###CamCtrlMouseSens", &mouse_sens_, 0.05f, 0.1f, 10.0f, "%.2f")) {
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Move Speed");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat("###CamCtrlMoveSpeed", &move_speed_, 0.1f, 0.1f, 100.0f, "%.2f")) {
    changed = true;
  }

  ImGui::TableNextColumn();
  ImGui::Text("Sprint Multiplier");
  ImGui::TableNextColumn();

  if (ImGui::DragFloat("###CamCtrlSprintMul", &sprint_multiplier_, 0.1f, 1.0f, 10.0f, "%.2f")) {
    changed = true;
  }
}


auto CameraControllerComponent::Start() -> void {
  auto& window{App::Instance().GetWindow()};
  window.SetCursorLock(GetCursorPosition());
  window.SetCursorHiding(true);
}


auto CameraControllerComponent::Update() -> void {
  if (!GetEntity()->GetComponent<CameraComponent>()) {
    return;
  }

  auto& transform{GetEntity()->GetTransform()};

  Vector3 move_dir;

  if (GetKey(Key::D)) {
    move_dir += transform.GetRightAxis();
  }

  if (GetKey(Key::A)) {
    move_dir -= transform.GetRightAxis();
  }

  if (GetKey(Key::Space)) {
    move_dir[1] += 1;
  }

  if (GetKey(Key::LeftControl)) {
    move_dir[1] -= 1;
  }

  if (GetKey(Key::W)) {
    move_dir += transform.GetForwardAxis();
  }

  if (GetKey(Key::S)) {
    move_dir -= transform.GetForwardAxis();
  }

  if (Length(move_dir) != 0) {
    Normalize(move_dir);
  }

  if (GetKey(Key::LeftShift)) {
    move_dir *= sprint_multiplier_;
  }

  transform.Translate(move_dir * move_speed_ * timing::GetFrameTime());

  auto const [mouse_delta_x, mouse_delta_y]{GetMouseDelta()};
  transform.Rotate(Vector3::Up(), mouse_delta_x * mouse_sens_, Space::World);
  transform.Rotate(Vector3::Right(), mouse_delta_y * mouse_sens_, Space::Local);
}


CameraControllerComponent::CameraControllerComponent() {
  SetUpdatable(true);
}


auto CameraControllerComponent::get_mouse_sens() const -> float {
  return mouse_sens_;
}


auto CameraControllerComponent::set_mouse_sens(float const sens) -> void {
  mouse_sens_ = sens;
}


auto CameraControllerComponent::get_move_speed() const -> float {
  return move_speed_;
}


auto CameraControllerComponent::set_move_speed(float const speed) -> void {
  move_speed_ = speed;
}


auto CameraControllerComponent::get_sprint_multiplier() const -> float {
  return sprint_multiplier_;
}


auto CameraControllerComponent::set_sprint_multiplier(float const multiplier) -> void {
  sprint_multiplier_ = multiplier;
}
}
