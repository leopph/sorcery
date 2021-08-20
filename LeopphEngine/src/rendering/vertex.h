#pragma once

#include "../math/Vector.hpp"

namespace leopph::impl
{
	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 textureCoordinates;
	};
}