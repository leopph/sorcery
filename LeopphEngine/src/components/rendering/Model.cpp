#include "Model.hpp"

#include "../../data/DataManager.hpp"
#include "../../rendering/geometry/FileMeshDataGroup.hpp"

#include <utility>


namespace leopph
{
	Model::Model(leopph::Entity* const entity, std::filesystem::path path) :
		RenderComponent{entity, *GetMeshData()},
		m_Path{std::move(path)}
	{}

	const impl::MeshDataGroup* Model::GetMeshData() const
	{
		if (const auto p{impl::DataManager::FindMeshDataGroup(m_Path.string())}; 
			p != nullptr)
		{
			return p;
		}
		impl::DataManager::StoreMeshDataGroup(impl::FileMeshDataGroup{m_Path});
		return impl::DataManager::FindMeshDataGroup(m_Path.string());
	}

}
