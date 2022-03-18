#include "MeshDataGroup.hpp"

#include "../../data/DataManager.hpp"

#include <utility>


namespace leopph::internal
{
	MeshDataGroup::MeshDataGroup(std::string id) :
		m_Id{std::move(id)}
	{
		DataManager::Instance().RegisterMeshDataGroup(this);
	}


	MeshDataGroup::~MeshDataGroup() noexcept
	{
		DataManager::Instance().UnregisterMeshDataGroup(this);
	}
}
