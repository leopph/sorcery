#include "RenderableEqual.hpp"


namespace leopph::impl
{
	bool RenderableEqual::operator()(const GlMeshGroup& left, const GlMeshGroup& right) const
	{
		return left.MeshData().Id() == right.MeshData().Id();
	}

	bool RenderableEqual::operator()(const GlMeshGroup& left, const MeshDataGroup& right) const
	{
		return MeshDataGroup().Id() == right.Id();
	}

	bool RenderableEqual::operator()(const MeshDataGroup& left, const GlMeshGroup& right) const
	{
		return left.Id() == right.MeshData().Id();
	}
}