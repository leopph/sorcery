#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>


namespace demo
{
	template<class T>
	using Registry = std::unordered_map<std::string, T>;

	extern Registry<std::size_t> g_Registry;
}
