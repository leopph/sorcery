#pragma once

#include <functional>


namespace leopph::impl
{
	template<class T>
	struct FuncEqual
	{
		bool operator()(const std::function<T>& left, const std::function<T>& right) const
		{
			return left.template target<T>() == right.template target<T>();
		}
	};
}