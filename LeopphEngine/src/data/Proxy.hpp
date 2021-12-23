#pragma once

#include <memory>
#include <utility>


namespace leopph::internal
{
	template<class T>
	class Proxy
	{
		public:
			Proxy(auto&&... args) :
				m_Ptr{std::make_unique<T>(std::forward<decltype(args)>(args)...)},
				data{m_Ptr}
			{}

			Proxy(const Proxy& other) :
				m_Ptr{std::make_unique<T>(*other.m_Ptr)},
				data{m_Ptr}
			{}

			Proxy(Proxy&& other) :
				m_Ptr{std::make_unique<T>(std::move(*other.m_Ptr))},
				data{m_Ptr}
			{}

			auto operator=(const Proxy& other) -> Proxy&
			{
				m_Ptr = std::make_unique<T>(*other.m_Ptr);
				return *this;
			}

			auto operator=(Proxy&& other) -> Proxy&
			{
				m_Ptr = std::make_unique<T>(std::move(*other.m_Ptr));
				return *this;
			}

			~Proxy() = default;

		private:
			std::unique_ptr<T> m_Ptr;

		protected:
			const std::unique_ptr<T>& data;
	};
}
