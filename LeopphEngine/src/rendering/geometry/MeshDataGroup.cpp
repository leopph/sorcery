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

	auto MeshDataGroup::Id() const -> const std::string&
	{
		return m_Id;
	}

	auto MeshDataGroup::Data() const -> const std::vector<MeshData>&
	{
		return m_MeshData;
	}

	auto MeshDataGroup::Data() -> std::vector<MeshData>&
	{
		return m_MeshData;
	}

	auto MeshDataGroup::GenerateId() noexcept -> std::string
	{
		static std::string idBase{"Leopph+Engine?Mesh&Data%Collection#"};
		static auto idCount{0ull};
		return idBase + std::to_string(idCount++);
	}
}
