#pragma once

#include "../../rendering/geometry/GlMeshCollection.hpp"
#include "../../rendering/geometry/MeshDataCollection.hpp"


namespace leopph::impl
{
	class RenderableEqual
	{
	public:
		using is_transparent = void;

		bool operator()(const GlMeshCollection& left, const GlMeshCollection& right) const;
		bool operator()(const GlMeshCollection& left, const MeshDataCollection& right) const;
		bool operator()(const MeshDataCollection& left, const GlMeshCollection& right) const;
	};
}