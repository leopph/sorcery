#include "GlMeshGroupHash.hpp"


namespace leopph::impl
{
	std::size_t GlMeshGroupHash::operator()(const GlMeshGroup& model) const
	{
		return m_Hash(model.MeshData().Id());
	}

	std::size_t GlMeshGroupHash::operator()(const std::shared_ptr<const MeshDataGroup>& meshDataGroup) const
	{
		return m_Hash(meshDataGroup->Id());
	}
}