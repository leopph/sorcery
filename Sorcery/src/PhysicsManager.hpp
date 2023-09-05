#pragma once

#include "Core.hpp"

#include <memory>


namespace sorcery {
class Component;


class InternalRigidBody {
  friend class PhysicsManager;
  void* mData;
};


class PhysicsManager {
  class Impl;
  std::unique_ptr<Impl> mImpl;

public:
  LEOPPHAPI PhysicsManager();
  PhysicsManager(PhysicsManager const&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;

  LEOPPHAPI ~PhysicsManager();

  auto operator=(PhysicsManager const&) -> void = delete;
  auto operator=(PhysicsManager&&) -> void = delete;

  LEOPPHAPI auto StartUp() const -> void;
  LEOPPHAPI auto ShutDown() const -> void;
  LEOPPHAPI auto Update() const -> void;

  [[nodiscard]] LEOPPHAPI auto CreateInternalRigidBody(ObserverPtr<Component> owningComponent) const -> ObserverPtr<InternalRigidBody>;
  LEOPPHAPI auto DestroyInternalRigidBody(ObserverPtr<InternalRigidBody> internalRigidBody) const -> void;
};


LEOPPHAPI extern PhysicsManager gPhysicsManager;
}
