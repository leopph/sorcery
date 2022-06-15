#pragma once

#include "Color.hpp"
#include "Matrix.hpp"

#include <assimp/types.h>


namespace leopph::convert
{
	auto Convert(aiMatrix4x4 const& aiMat) -> Matrix4;
	auto Convert(aiVector3D const& aiVec) -> Vector3;
	auto Convert(aiColor3D const& aiCol) -> Color;
}
