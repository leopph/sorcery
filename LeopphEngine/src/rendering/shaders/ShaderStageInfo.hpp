#pragma once

#include "ShaderType.hpp"

#include <string>


namespace leopph::internal
{
	struct ShaderStageInfo
	{
		std::string Src;
		ShaderType Type;
	};
}
