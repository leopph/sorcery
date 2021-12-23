#include "ShaderTypeEqual.hpp"


namespace leopph::internal
{
	auto ShaderTypeEqual::operator()(ShaderType left, ShaderType right) const -> bool
	{
		return left == right;
	}

	auto ShaderTypeEqual::operator()(const int left, const int right) const -> bool
	{
		return left == right;
	}
}
