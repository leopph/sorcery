#include "GlMeshGroupEqual.hpp"


namespace leopph::internal
{
	auto GlMeshGroupEqual::operator()(const GlMeshGroup& left, const GlMeshGroup& right) const -> bool
	{
		return left.MeshData().Id() == right.MeshData().Id();
	}

	auto GlMeshGroupEqual::operator()(const GlMeshGroup& left, const std::shared_ptr<const MeshDataGroup>& right) const -> bool
	{
		return left.MeshData().Id() == right->Id();
	}

	auto GlMeshGroupEqual::operator()(const std::shared_ptr<const MeshDataGroup>& left, const GlMeshGroup& right) const -> bool
	{
		return left->Id() == right.MeshData().Id();
	}
}
