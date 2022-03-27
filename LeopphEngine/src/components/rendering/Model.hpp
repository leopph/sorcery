#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"

#include <filesystem>
#include <memory>


namespace leopph
{
	// A graphic model parsed from a file on disk.
	class Model final : public internal::RenderComponent
	{
		public:
			// Load a Model from a file on disk.
			LEOPPHAPI explicit Model(std::filesystem::path path);

			// File path of the loaded Model.
			[[nodiscard]] LEOPPHAPI
			auto Path() const noexcept -> std::filesystem::path const&;

			LEOPPHAPI ~Model() override = default;

		private:
			[[nodiscard]] auto GetMeshGroup(std::filesystem::path const& path) const -> std::shared_ptr<internal::MeshGroup const>;

			std::filesystem::path m_Path;
	};
}
