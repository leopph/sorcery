#include "RenderableEqual.hpp"


namespace leopph::impl
{
	bool RenderableEqual::operator()(const GlMeshGroup& left, const GlMeshGroup& right) const
	{
		return left.MeshDataCollection().Id() == right.MeshDataCollection().Id();
	}

	bool RenderableEqual::operator()(const GlMeshGroup& left, const MeshDataGroup& right) const
	{
		return MeshDataGroup().Id() == right.Id();
	}

	bool RenderableEqual::operator()(const MeshDataGroup& left, const GlMeshGroup& right) const
	{
		return left.Id() == right.MeshDataCollection().Id();
	}
}