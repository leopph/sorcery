#pragma once

#include "Component.hpp"
#include "PhysicsManager.hpp"


namespace sorcery {
class RigidBody : public Component {
public:
  LEOPPHAPI static Type const SerializationType;

  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
  LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
  LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

  LEOPPHAPI RigidBody();
  LEOPPHAPI ~RigidBody() override;

private:
  RTTR_ENABLE(Component)
  ObserverPtr<InternalRigidBody> mInternalPtr;
};
}
