#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	class Model final : public impl::RenderComponent
	{
		public:
			// Load a Model from a file on disk.
			LEOPPHAPI Model(leopph::Entity* entity, std::filesystem::path path);

			Model(const Model& other) = delete;
			Model& operator=(const Model& other) = delete;

			Model(Model&& other) = delete;
			Model& operator=(Model&& other) = delete;

			LEOPPHAPI ~Model() override = default;

			// File path of the loaded Model.
			const std::filesystem::path& Path() const;

		private:
			std::filesystem::path m_Path;

			const impl::MeshDataGroup* GetMeshData() const;
	};
}
