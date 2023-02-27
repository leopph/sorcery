#pragma once

#include "Component.hpp"

namespace leopph {
class BehaviorComponent : public Component {
public:
	LEOPPHAPI ~BehaviorComponent() override;

	LEOPPHAPI auto CreateAndInitManagedObject(MonoClass* klass) -> void;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
	LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

	// NOP on Behaviors. Use CreateAndInitManagedObject.
	LEOPPHAPI auto CreateManagedObject() -> void override;
};

LEOPPHAPI auto init_behaviors() -> void;
LEOPPHAPI auto tick_behaviors() -> void;
LEOPPHAPI auto tack_behaviors() -> void;
}
