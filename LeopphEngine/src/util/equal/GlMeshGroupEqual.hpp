#pragma once

#include "../../rendering/geometry/GlMeshGroup.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"


namespace leopph::impl
{
	class GlMeshGroupEqual
	{
	public:
		using is_transparent = void;

		bool operator()(const GlMeshGroup& left, const GlMeshGroup& right) const;
		bool operator()(const GlMeshGroup& left, const MeshDataGroup& right) const;
		bool operator()(const MeshDataGroup& left, const GlMeshGroup& right) const;
	};
}