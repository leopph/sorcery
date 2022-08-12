#pragma once

/*#include "LeopphApi.hpp"
#include "StaticModelComponent.hpp"

#include <filesystem>


namespace leopph
{
	// A 2D image parsed from disk, displayed as a 2D sprite.
	class ImageSprite final : public internal::StaticModelComponent
	{
		public:
			// PPI must be positive.
			explicit LEOPPHAPI ImageSprite(std::filesystem::path const& src, int ppi = 1);

			[[nodiscard]] LEOPPHAPI std::filesystem::path const& Path() const noexcept;

			// Get the extents of the ImageSprite.
			// This is half the size of the ImageSprite.
			[[nodiscard]] LEOPPHAPI Vector2 const& Extents() const noexcept;

		private:
			[[nodiscard]] static std::shared_ptr<StaticMesh> create_data(Image& img, int ppi);

			std::filesystem::path m_Path;
			Vector2 m_Extents;
	};
}
*/