#pragma once

#include "Core.hpp"


namespace sorcery {
class PhysicsManager {
  class Impl;

  Impl* mImpl;

public:
  LEOPPHAPI PhysicsManager();
  PhysicsManager(PhysicsManager const&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;

  LEOPPHAPI ~PhysicsManager();

  auto operator=(PhysicsManager const&) -> void = delete;
  auto operator=(PhysicsManager&&) -> void = delete;

  LEOPPHAPI void StartUp() const;
  LEOPPHAPI void ShutDown() const;
};
}
