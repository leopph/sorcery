#include "MeshDataGroup.hpp"

#include <utility>


namespace leopph::impl
{
	MeshDataGroup::MeshDataGroup(std::string id) :
		m_Id{std::move(id)}
	{}

	const std::string& MeshDataGroup::Id() const
	{
		return m_Id;
	}

	const std::vector<MeshData>& MeshDataGroup::Data() const
	{
		return *m_MeshData;
	}

	std::vector<MeshData>& MeshDataGroup::Data()
	{
		return *m_MeshData;
	}


	std::string MeshDataGroup::GenerateId() noexcept
	{
		static std::string idBase{"Leopph+Engine?Mesh&Data%Collection#"};
		static auto idCount{0ull};
		return idBase + std::to_string(idCount++);
	}
}
