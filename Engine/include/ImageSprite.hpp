#pragma once

#include "LeopphApi.hpp"
#include "RenderComponent.hpp"
#include "MeshGroup.hpp"

#include <filesystem>


namespace leopph
{
	// A 2D image parsed from disk, displayed as a 2D sprite.
	class ImageSprite final : public internal::RenderComponent
	{
		public:
			// PPI must be positive.
			explicit LEOPPHAPI ImageSprite(std::filesystem::path const& src, int ppi = 1);

			[[nodiscard]] auto LEOPPHAPI Clone() const -> ComponentPtr<> override;

			[[nodiscard]] auto LEOPPHAPI Path() const noexcept -> std::filesystem::path const&;

			// Get the extents of the ImageSprite.
			// This is half the size of the ImageSprite.
			[[nodiscard]] auto LEOPPHAPI Extents() const noexcept -> Vector2 const&;

		private:
			[[nodiscard]] static auto CreateMeshGroup(Image& img, int ppi) -> MeshGroup;

			std::filesystem::path m_Path;
			Vector2 m_Extents;
	};
}
