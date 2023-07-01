#pragma once

#include "Core.hpp"
#include "Util.hpp"


namespace sorcery {
class InternalStaticRigidBody {
  friend class PhysicsManager;
  void* mData;
};


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

  LEOPPHAPI auto StartUp() const -> void;
  LEOPPHAPI auto ShutDown() const -> void;
  LEOPPHAPI auto Update() const -> void;

  [[nodiscard]] LEOPPHAPI auto CreateInternalStaticRigidBody() const -> ObserverPtr<InternalStaticRigidBody>;
  LEOPPHAPI auto DestroyInternalStaticRigidBody(ObserverPtr<InternalStaticRigidBody> internalStaticRigidBody) const -> void;
};
}
