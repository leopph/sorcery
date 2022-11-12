#pragma once

#include "ManagedAccessObject.hpp"

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include <span>

namespace leopph {
	class SceneElement : public ManagedAccessObject {
	public:
		enum class Type {
			Entity, Transform, Camera, Behavior, CubeModel
		};

		LEOPPHAPI SceneElement();
		LEOPPHAPI ~SceneElement() override;

		
		[[nodiscard]] virtual auto GetSerializationType() const -> Type = 0;
		virtual auto Serialize(YAML::Node& node) const -> void = 0;
		virtual auto Deserialize(YAML::Node const& node) -> void = 0;
	};

	[[nodiscard]] LEOPPHAPI auto GetAllSceneElements() -> std::span<SceneElement* const>;
}