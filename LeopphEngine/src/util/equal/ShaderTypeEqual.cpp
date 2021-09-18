#include "ShaderTypeEqual.hpp"


namespace leopph::impl
{
	bool ShaderTypeEqual::operator()(ShaderType left, ShaderType right) const
	{
		return left == right;
	}


	bool ShaderTypeEqual::operator()(const int left, const int right) const
	{
		return left == right;
	}
}