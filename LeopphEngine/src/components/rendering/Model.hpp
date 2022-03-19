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

			Model(Model const& other) = delete;
			auto operator=(Model const& other) -> Model& = delete;

			Model(Model&& other) = delete;
			auto operator=(Model&& other) -> Model& = delete;

			LEOPPHAPI ~Model() override = default;

			// File path of the loaded Model.
			[[nodiscard]] constexpr auto Path() const noexcept -> auto&;

		private:
			[[nodiscard]] auto GetMeshGroup(std::filesystem::path const& path) const -> std::shared_ptr<internal::MeshGroup const>;

			std::filesystem::path m_Path;
	};


	constexpr auto Model::Path() const noexcept -> auto&
	{
		return m_Path;
	}
}
