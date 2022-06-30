#include "MeshGroup.hpp"

#include "Logger.hpp"

#include <utility>


namespace leopph
{
	MeshGroup::MeshGroup(std::span<Mesh const> meshes) :
		m_Meshes{std::begin(meshes), std::end(meshes)}
	{}


	MeshGroup::MeshGroup(std::vector<Mesh> meshes) :
		m_Meshes{std::move(meshes)}
	{}


	auto MeshGroup::Meshes() const noexcept -> std::span<Mesh const>
	{
		return m_Meshes;
	}


	auto MeshGroup::AddMesh(Mesh mesh) -> void
	{
		m_Meshes.push_back(std::move(mesh));
	}


	auto MeshGroup::RemoveMesh(u64 const index) -> void
	{
		if (index >= m_Meshes.size())
		{
			internal::Logger::Instance().Error("Ignoring attempt to remove Mesh from MeshGroup at index " + std::to_string(index) + ". MeshGroup size is " + std::to_string(m_Meshes.size()) + '.');
			return;
		}

		m_Meshes.erase(std::cbegin(m_Meshes) + index);
	}
}
