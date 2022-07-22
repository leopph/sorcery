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


	std::span<Mesh const> MeshGroup::Meshes() const noexcept
	{
		return m_Meshes;
	}


	void MeshGroup::AddMesh(Mesh mesh)
	{
		m_Meshes.push_back(std::move(mesh));
	}


	void MeshGroup::RemoveMesh(u64 const index)
	{
		if (index >= m_Meshes.size())
		{
			internal::Logger::Instance().Error("Ignoring attempt to remove Mesh from MeshGroup at index " + std::to_string(index) + ". MeshGroup size is " + std::to_string(m_Meshes.size()) + '.');
			return;
		}

		m_Meshes.erase(std::cbegin(m_Meshes) + index);
	}
}
