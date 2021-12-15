#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>


namespace leopph::impl
{
	template<class T>
	struct IsSharedPtr : std::false_type
	{};


	template<class T>
	struct IsSharedPtr<std::shared_ptr<T>> : std::true_type
	{};


	template<class T>
	struct IsUniquePtr : std::false_type
	{};


	template<class T>
	struct IsUniquePtr<std::unique_ptr<T>> : std::true_type
	{};


	template<class T>
	concept SharedPtr = IsSharedPtr<T>::value;

	template<class T>
	concept UniquePtr = IsUniquePtr<T>::value;

	template<class T>
	concept RawPtr = std::is_pointer_v<T>;

	template<class T>
	concept NullPtr = std::same_as<T, std::nullptr_t>;

	template<class T>
	concept Pointer = SharedPtr<T> || UniquePtr<T> || RawPtr<T> || NullPtr<T>;
}
