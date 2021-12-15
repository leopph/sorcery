#pragma once

#include "../concepts/Idable.hpp"
#include "../concepts/Pointer.hpp"

#include <functional>
#include <memory>
#include <string>


namespace leopph::impl
{
	class IdHash
	{
		public:
			using is_transparent = void;

			template<Idable T>
			std::size_t operator()(const T& obj) const
			{
				return m_Hash(obj.Id());
			}

			template<class T> requires Idable<T> && Pointer<T>
			std::size_t operator()(const T& obj) const
			{
				return m_Hash(obj->Id());
			}

			std::size_t operator()(const std::string& str) const
			{
				return m_Hash(str);
			}

		private:
			std::hash<std::string> m_Hash;
	};
}
