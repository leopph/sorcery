#pragma once

#include "../../math/Vector.hpp"


namespace leopph::impl
{
	struct Vertex
	{
		Vector3 Position{};
		Vector3 Normal{};
		Vector2 TexCoord{};
	};
}
