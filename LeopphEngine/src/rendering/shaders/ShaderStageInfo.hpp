#pragma once

#include "ShaderType.hpp"

#include <string>


namespace leopph::internal
{
	struct ShaderStageInfo
	{
		std::string src;
		ShaderType type;
	};
}