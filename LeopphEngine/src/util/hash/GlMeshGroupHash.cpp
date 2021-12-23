#include "GlMeshGroupHash.hpp"


namespace leopph::internal
{
	auto GlMeshGroupHash::operator()(const GlMeshGroup& model) const -> std::size_t
	{
		return m_Hash(model.MeshData().Id());
	}

	auto GlMeshGroupHash::operator()(const std::shared_ptr<const MeshDataGroup>& meshDataGroup) const -> std::size_t
	{
		return m_Hash(meshDataGroup->Id());
	}
}
