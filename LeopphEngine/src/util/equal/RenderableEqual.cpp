#include "RenderableEqual.hpp"


namespace leopph::impl
{
	bool RenderableEqual::operator()(const GlMeshCollection& left, const GlMeshCollection& right) const
	{
		return left.MeshDataCollection().Id() == right.MeshDataCollection().Id();
	}

	bool RenderableEqual::operator()(const GlMeshCollection& left, const MeshDataCollection& right) const
	{
		return MeshDataCollection().Id() == right.Id();
	}

	bool RenderableEqual::operator()(const MeshDataCollection& left, const GlMeshCollection& right) const
	{
		return left.Id() == right.MeshDataCollection().Id();
	}
}