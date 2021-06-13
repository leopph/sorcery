#pragma once

#include "../math/vector.h"

namespace leopph::impl
{
	// STRUCT TO REPRESENT A VERTEX
	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 textureCoordinates;
	};
}