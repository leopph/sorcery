#include "Cube.hpp"

std::vector<Vector3> cubePositions;

leopph::u64 add_cube_pos(Vector3 const* pos)
{
	cubePositions.emplace_back(*pos);
	return cubePositions.size() - 1;
}

void update_cube_pos(leopph::u64 const index, Vector3 const* pos)
{
	cubePositions[index] = *pos;
}