#pragma once

#include "Component.hpp"
#include "../PhysicsManager.hpp"


namespace sorcery {
class RigidBodyComponent : public Component {
public:
  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;

private:
  RTTR_ENABLE(Component)
  ObserverPtr<InternalRigidBody> mInternalPtr{nullptr};
};
}
