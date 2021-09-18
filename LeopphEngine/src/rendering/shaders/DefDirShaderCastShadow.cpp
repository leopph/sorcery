#include "DefDirShaderCastShadow.hpp"

#include "ShaderStage.hpp"


namespace leopph::impl
{
	DefDirShaderCastShadow::DefDirShaderCastShadow() :
		DefDirShader{std::vector<std::string>{}, std::vector<std::string>{"CAST_SHADOW"}}
	{}

}