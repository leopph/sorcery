#pragma once

#include "LeopphApi.hpp"
#include "StaticModelData.hpp"

#include <filesystem>
#include <memory>
#include <utility>
#include <vector>


namespace leopph
{
	[[nodiscard]] LEOPPHAPI StaticModelData import_static_model(std::filesystem::path const& path);

	class StaticMesh;
	class StaticMaterial;

	[[nodiscard]] std::vector<std::pair<std::shared_ptr<StaticMesh>, std::shared_ptr<StaticMaterial>>> generate_render_structures(StaticModelData const& modelData);
}
