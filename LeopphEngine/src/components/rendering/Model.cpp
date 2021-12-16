#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/FileMeshDataGroup.hpp"

#include <utility>


namespace leopph
{
	Model::Model(leopph::Entity* const entity, std::filesystem::path path) :
		RenderComponent{entity, *GetMeshData(path)},
		m_Path{std::move(path)}
	{}

	const impl::MeshDataGroup* Model::GetMeshData(const std::filesystem::path& path) const
	{
		if (const auto p{impl::DataManager::FindMeshDataGroup(path.generic_string())}; 
			p != nullptr)
		{
			return p;
		}
		impl::DataManager::StoreMeshDataGroup(impl::FileMeshDataGroup{path});
		return impl::DataManager::FindMeshDataGroup(path.generic_string());
	}

}
