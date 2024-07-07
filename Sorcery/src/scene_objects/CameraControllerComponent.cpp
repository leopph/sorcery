#include "CameraControllerComponent.hpp"

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
      &sorcery::CameraControllerComponent::set_move_speed);
}


namespace sorcery {
auto CameraControllerComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<CameraControllerComponent>(*this);
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

  Vector3 move_dir;

  if (GetKey(Key::D)) {
    move_dir[0] += 1;
  }

  if (GetKey(Key::A)) {
    move_dir[0] -= 1;
  }

  if (GetKey(Key::Space)) {
    move_dir[1] += 1;
  }

  if (GetKey(Key::LeftControl)) {
    move_dir[1] -= 1;
  }

  if (GetKey(Key::W)) {
    move_dir[2] += 1;
  }

  if (GetKey(Key::S)) {
    move_dir[2] -= 1;
  }

  if (Length(move_dir) != 0) {
    Normalize(move_dir);
  }

  if (GetKey(Key::LeftShift)) {
    move_dir *= 2;
  }

  auto& transform{GetEntity()->GetTransform()};
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
}
