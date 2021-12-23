#pragma once

#include <concepts>
#include <string>
#include <type_traits>


namespace leopph::internal
{
	template<class T1, class T2>
	concept SameOrReferenceTo = std::same_as<std::remove_cvref_t<T1>, std::remove_cvref_t<T2>>;

	template<class T>
	concept Idable = requires(const T& obj)
	{
		{
			obj.Id()
		} -> SameOrReferenceTo<std::string>;
	} || requires(const T& obj)
	{
		{
			obj->Id()
		} -> SameOrReferenceTo<std::string>;
	};
}
