#pragma once

#include "ShaderType.hpp"

#include <string>


namespace leopph::impl
{
	struct ShaderStageInfo
	{
		std::string src;
		ShaderType type;
	};
}