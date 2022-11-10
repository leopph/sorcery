// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)


namespace leopph::editor {
	[[nodiscard]] auto SerializeScene() -> YAML::Node;
	auto DeserializeScene(YAML::Node const& scene) -> void;
}