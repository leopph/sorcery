#pragma once

#include "AABB.hpp"
#include "Types.hpp"


namespace leopph
{
	struct SubMeshDescriptor
	{
		i32 indexOffset;
		i32 indexCount;
		i32 baseVertex;
		AABB boundingBox;
	};
}
