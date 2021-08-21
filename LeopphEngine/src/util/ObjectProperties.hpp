#pragma once

#include "../math/Quaternion.hpp"
#include "../math/Vector.hpp"

#include <string>

namespace leopph
{
	struct ObjectProperties
	{
		std::string name{};
		bool isStatic{ false };
		Vector3 position{};
		Quaternion rotation{};
		Vector3 scale{ 1, 1, 1 };
	};
}