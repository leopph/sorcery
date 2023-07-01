#pragma once

#include "Component.hpp"
#include "PhysicsManager.hpp"


namespace sorcery {
class StaticRigidBody : public Component {
public:
  LEOPPHAPI static Type const SerializationType;

  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
  LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
  LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

  LEOPPHAPI StaticRigidBody();
  LEOPPHAPI ~StaticRigidBody() override;

private:
  RTTR_ENABLE(Component)
  ObserverPtr<InternalStaticRigidBody> mInternalPtr;
};
}
