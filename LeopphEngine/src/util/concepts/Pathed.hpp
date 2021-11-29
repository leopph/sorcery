#pragma once

#include <concepts>
#include <filesystem>


namespace leopph::impl
{

	template<class T>
	concept Pathed = requires(T obj)
	{
		{
			obj.Path
		} -> std::convertible_to<const std::filesystem::path>;
	};
}