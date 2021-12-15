#pragma once

#include <concepts>
#include <string>


namespace leopph::impl
{
	template<class T>
	concept Idable = requires(const T& obj)
	{
		{
			obj.Id()
		} -> std::same_as<std::string>;
	} || requires(const T& obj)
	{
		{
			obj->Id()
		} -> std::same_as<std::string>;
	};
}
