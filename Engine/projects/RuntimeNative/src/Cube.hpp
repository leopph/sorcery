#pragma once

#include "Core.hpp"

#include <vector>

struct Vector3
{
	float x, y, z;
};


LEOPPHAPI extern std::vector<Vector3> cubePositions;
leopph::u64 add_cube_pos(Vector3 const* pos);
void update_cube_pos(leopph::u64 index, Vector3 const* pos);