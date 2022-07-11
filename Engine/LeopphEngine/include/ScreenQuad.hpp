#pragma once

#include "Types.hpp"

#include <array>


namespace leopph::internal
{
	// Vertex positions of a quad that fills the entire screen.
	// Coordinates are normalized to [-1, 1].
	// The vertices are organized so that they form a quad when rendered as a triangle strip.
	extern std::array<f32,8> const g_ScreenQuadVertices; 
}
