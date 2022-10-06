#pragma once

#include "Math.hpp"

#include <string>


namespace leopph
{
	struct Node
	{
		u64 id{};
		std::wstring name{};
		Vector3 position{0};
	};
}