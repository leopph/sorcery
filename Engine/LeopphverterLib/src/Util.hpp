#pragma once

#include <type_traits>


namespace leopph::convert
{
	template<class T>
	concept Scalar = std::is_scalar_v<T>;
}
