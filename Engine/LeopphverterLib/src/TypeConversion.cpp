#include "TypeConversion.hpp"


namespace leopph::convert
{
	auto Convert(aiMatrix4x4 const& aiMat) -> Matrix4
	{
		return Matrix4
		{
			aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
			aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
			aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
			aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
		};
	}


	auto Convert(aiVector3D const& aiVec) -> Vector3
	{
		return Vector3{aiVec.x, aiVec.y, aiVec.z};
	}


	auto Convert(aiColor3D const& aiCol) -> Color
	{
		return Color{static_cast<unsigned char>(aiCol.r * 255), static_cast<unsigned char>(aiCol.g * 255), static_cast<unsigned char>(aiCol.b * 255)};
	}
}
