#pragma once

#include "../concepts/Idable.hpp"
#include "../concepts/Pointer.hpp"

#include <functional>
#include <memory>
#include <string>


namespace leopph::internal
{
	class IdHash
	{
		public:
			using is_transparent = void;

			template<class T>
				requires Idable<T> && NotPointer<T>
			auto operator()(const T& obj) const -> std::size_t
			{
				return m_Hash(obj.Id());
			}

			template<class T>
				requires Idable<T> && Pointer<T>
			auto operator()(const T& obj) const -> std::size_t
			{
				return m_Hash(obj->Id());
			}

			auto operator()(const std::string& str) const -> std::size_t
			{
				return m_Hash(str);
			}

		private:
			std::hash<std::string> m_Hash;
	};
}
