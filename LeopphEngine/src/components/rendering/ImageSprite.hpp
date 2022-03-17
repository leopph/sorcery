#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"

#include <filesystem>
#include <string>


namespace leopph
{
	// A 2D image parsed from disk, displayed as a 2D sprite.
	class ImageSprite final : public internal::RenderComponent
	{
		public:
			LEOPPHAPI explicit ImageSprite(const std::filesystem::path& src, unsigned ppi = 1);

		private:
			[[nodiscard]] static
			auto GetMeshData(const std::filesystem::path& src, unsigned ppi) -> std::shared_ptr<const internal::MeshDataGroup>;


			class MeshDataGroup final : public internal::MeshDataGroup
			{
				public:
					explicit MeshDataGroup(const std::filesystem::path& src, unsigned ppi);

					// Creates the used mesh id from the source path and ppi.
					[[nodiscard]] static
					auto GetMeshId(const std::filesystem::path& src, unsigned ppi) -> std::string;
			};
	};
}
