#pragma once

#include <glm/glm.hpp>

namespace leopph
{
	// STRUCT TO REPRESENT A VERTEX WITH ALL ITS DATA
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoordinates;
	};
}