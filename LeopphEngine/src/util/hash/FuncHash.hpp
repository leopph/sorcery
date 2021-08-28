#pragma once

#include <functional>


namespace leopph::impl
{
	template<class T>
	struct FuncHash
	{
		bool operator()(const std::function<T>& func) const
		{
			return std::hash<const T*>{}(func.template target<T>());
		}
	};
}