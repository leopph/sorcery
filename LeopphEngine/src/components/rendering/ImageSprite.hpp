#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"

#include <filesystem>
#include <string>


namespace leopph
{
	// A 2D image parsed from disk, displayed as a 2D sprite.
	class ImageSprite final : public internal::RenderComponent
	{
		public:
			// Load image from disk.
			// PPI must be positive.
			LEOPPHAPI explicit ImageSprite(std::filesystem::path const& src, int ppi = 1);

		private:
			[[nodiscard]] static
			auto GetMeshGroup(std::filesystem::path const& src, int ppi) -> std::shared_ptr<internal::MeshGroup const>;

			// Creates the used mesh id from the source path and ppi.
			[[nodiscard]] static
			auto GetMeshId(std::filesystem::path const& src, int ppi) -> std::string;
	};
}
