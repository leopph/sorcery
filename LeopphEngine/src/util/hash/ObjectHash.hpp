#pragma once

#include "../../hierarchy/Object.hpp"

#include <cstddef>
#include <functional>
#include <string>


namespace leopph::impl
{
	class ObjectHash
	{
	public:
		using is_transparent = void;

		ObjectHash();

		std::size_t operator()(const Object* object) const;
		std::size_t operator()(const std::string& str) const;


	private:
		std::hash<std::string> m_Hash;
	};
}