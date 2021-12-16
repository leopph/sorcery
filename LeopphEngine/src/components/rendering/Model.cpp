#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/FileMeshDataGroup.hpp"

#include <utility>


namespace leopph
{
	Model::Model(leopph::Entity* const entity, std::filesystem::path path) :
		RenderComponent{entity, GetMeshData(path)},
		m_Path{std::move(path)}
	{}

	std::shared_ptr<impl::MeshDataGroup> Model::GetMeshData(const std::filesystem::path& path) const
	{
		if (const auto p{impl::DataManager::FindMeshDataGroup(path.generic_string())};
			p != nullptr)
		{
			return p;
		}
		return std::make_shared<impl::FileMeshDataGroup>(path.generic_string());
	}

	const std::filesystem::path& Model::Path() const
	{
		return m_Path;
	}
}
