#include "MeshGroup.hpp"

#include "../../data/DataManager.hpp"

#include <utility>


namespace leopph::internal
{
	MeshGroup::MeshGroup(std::string id) :
		m_Id{std::move(id)}
	{
		DataManager::Instance().RegisterMeshGroup(this);
	}


	MeshGroup::~MeshGroup() noexcept
	{
		DataManager::Instance().UnregisterMeshGroup(this);
	}
}
