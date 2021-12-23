#pragma once

#include "../concepts/Idable.hpp"
#include "../concepts/Pointer.hpp"


namespace leopph::internal
{
	class IdEqual
	{
		public:
			using is_transparent = void;

			// For references

			template<class T>
				requires Idable<T> && NotPointer<T>
			auto operator()(const T& left, const T& right) const -> bool
			{
				return left.Id() == right.Id();
			}

			template<class T>
				requires Idable<T> && NotPointer<T>
			auto operator()(const T& left, const std::string& right) const -> bool
			{
				return left.Id() == right;
			}

			template<class T>
				requires Idable<T> && NotPointer<T>
			auto operator()(const std::string& left, const T& right) const -> bool
			{
				return left == right.Id();
			}

			// For pointers

			template<class T>
				requires Idable<T> && Pointer<T>
			auto operator()(const T& left, const T& right) const -> bool
			{
				return left->Id() == right->Id();
			}

			template<class T>
				requires Idable<T> && Pointer<T>
			auto operator()(const T& left, const std::string& right) const -> bool
			{
				return left->Id() == right;
			}

			template<class T>
				requires Idable<T> && Pointer<T>
			auto operator()(const std::string& left, const T& right) const -> bool
			{
				return left == right->Id();
			}
	};
}
