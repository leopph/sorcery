#include "RenderableHash.hpp"


namespace leopph::impl
{
	std::size_t RenderableHash::operator()(const GlMeshGroup& model) const
	{
		return m_Hash(model.MeshDataCollection().Id());
	}

	std::size_t RenderableHash::operator()(const MeshDataGroup& modelData) const
	{
		return m_Hash(modelData.Id());
	}
}