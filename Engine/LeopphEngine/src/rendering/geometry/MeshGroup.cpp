#include "MeshGroup.hpp"

#include "Logger.hpp"


namespace leopph
{
	MeshGroup::MeshGroup() :
		m_Meshes{std::make_shared<std::vector<Mesh>>()}
	{}


	MeshGroup::MeshGroup(std::vector<Mesh> meshes) :
		m_Meshes{std::make_shared<std::vector<Mesh>>(std::move(meshes))}
	{}


	MeshGroup::MeshGroup(std::shared_ptr<std::vector<Mesh>> meshes) :
		m_Meshes{std::move(meshes)}
	{
		if (!m_Meshes)
		{
			internal::Logger::Instance().Warning("Ignoring nullptr during MeshGroup construction. Reverting to default construction.");
			m_Meshes = std::make_shared<std::vector<Mesh>>();
		}
	}


	auto MeshGroup::Meshes() const noexcept -> std::span<Mesh const>
	{
		return GetMeshesCommon(this);
	}


	auto MeshGroup::Meshes() noexcept -> std::vector<Mesh>&
	{
		return GetMeshesCommon(this);
	}


	auto MeshGroup::UseCount() const noexcept -> std::size_t
	{
		return m_Meshes.use_count();
	}
}
