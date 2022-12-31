#pragma once

#include "Component.hpp"

namespace leopph {
	class BehaviorComponent : public Component {
	public:
		BehaviorComponent() = default;
		explicit BehaviorComponent(MonoClass* klass);
		LEOPPHAPI ~BehaviorComponent() override;

		LEOPPHAPI auto Init(MonoClass* klass) -> void;

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const->Type override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;
	};

	LEOPPHAPI auto init_behaviors() -> void;
	LEOPPHAPI auto tick_behaviors() -> void;
	LEOPPHAPI auto tack_behaviors() -> void;
}