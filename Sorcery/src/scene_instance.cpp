#include "sorcery/scene_instance.hpp"


namespace sorcery {
SceneInstanceId::SceneInstanceId(std::uint32_t const value) noexcept :
  value_{value} {}


auto SceneInstanceId::HasScene() const noexcept -> bool {
  return value_ != 0;
}


auto SceneInstanceId::Value() const noexcept -> std::uint32_t {
  return value_;
}


SceneInstanceId::operator unsigned int() const noexcept {
  return Value();
}
}
