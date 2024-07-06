#pragma once

#include "Component.hpp"


namespace sorcery {
class Behavior : public virtual Component {
  RTTR_ENABLE(Component)

public:
  virtual auto Update() -> void {}

protected:
  Behavior() = default;
  Behavior(Behavior const& other) = default;
  Behavior(Behavior&& other) noexcept = default;

public:
  ~Behavior() override = default;

  auto operator=(Behavior const& other) -> void = delete;
  auto operator=(Behavior&& other) -> void = delete;
};
}
