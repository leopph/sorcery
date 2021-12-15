#include "MeshDataCollection.hpp"

#include <utility>


namespace leopph::impl
{
	MeshDataCollection::MeshDataCollection(std::string id) :
		m_Id{std::move(id)}
	{}

	const std::string& MeshDataCollection::Id() const
	{
		return m_Id;
	}

	const std::vector<MeshData>& MeshDataCollection::Data() const
	{
		return *m_MeshData;
	}

	std::vector<MeshData>& MeshDataCollection::Data()
	{
		return *m_MeshData;
	}


	std::string MeshDataCollection::GenerateId() noexcept
	{
		static std::string idBase{"Leopph+Engine?Mesh&Data%Collection#"};
		static auto idCount{0ull};
		return idBase + std::to_string(idCount++);
	}
}
