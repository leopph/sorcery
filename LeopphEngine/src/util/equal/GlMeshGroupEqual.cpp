#include "GlMeshGroupEqual.hpp"


namespace leopph::impl
{
	bool GlMeshGroupEqual::operator()(const GlMeshGroup& left, const GlMeshGroup& right) const
	{
		return left.MeshData().Id() == right.MeshData().Id();
	}

	bool GlMeshGroupEqual::operator()(const GlMeshGroup& left, const std::shared_ptr<const MeshDataGroup>& right) const
	{
		return left.MeshData().Id() == right->Id();
	}

	bool GlMeshGroupEqual::operator()(const std::shared_ptr<const MeshDataGroup>& left, const GlMeshGroup& right) const
	{
		return left->Id() == right.MeshData().Id();
	}
}