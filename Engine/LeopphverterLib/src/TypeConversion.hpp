#pragma once

#include "Color.hpp"
#include "Matrix.hpp"

#include <assimp/types.h>


namespace leopph::convert
{
	Matrix4 Convert(aiMatrix4x4 const& aiMat);
	Vector3 Convert(aiVector3D const& aiVec);
	Color Convert(aiColor3D const& aiCol);
}
