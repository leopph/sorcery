#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"

#include <filesystem>


namespace leopph
{
	// A 2D image parsed from disk, displayed as a 2D sprite.
	class ImageSprite final : public internal::RenderComponent
	{
		public:
			// PPI must be positive.
			LEOPPHAPI explicit ImageSprite(std::filesystem::path const& src, int ppi = 1);

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			[[nodiscard]] LEOPPHAPI
			auto Path() const noexcept -> std::filesystem::path const&;

			// Get the extents of the ImageSprite.
			// This is half the size of the ImageSprite.
			[[nodiscard]] LEOPPHAPI
			auto Extents() const noexcept -> Vector2 const&;

		private:
			[[nodiscard]]
			auto CreateMeshGroup(Image& img, int ppi) -> MeshGroup;

			std::filesystem::path m_Path;
			Vector2 m_Extents;
	};
}
