#pragma once

#include "Component.hpp"
#include "PhysicsManager.hpp"


namespace sorcery {
class RigidBodyComponent : public Component {
public:
  LEOPPHAPI static Type const SerializationType;

  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

  LEOPPHAPI RigidBodyComponent();
  LEOPPHAPI ~RigidBodyComponent() override;

private:
  RTTR_ENABLE(Component)
  ObserverPtr<InternalRigidBody> mInternalPtr;
};
}
