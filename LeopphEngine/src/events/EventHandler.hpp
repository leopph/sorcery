#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>


namespace leopph
{
	template<class... T>
	class EventHandler
	{
		public:
			using CallbackType = std::function<void(std::decay_t<T>...)>;

			explicit EventHandler(CallbackType function);
			EventHandler(const EventHandler<T...>& other);
			EventHandler(EventHandler<T...>&& other) = delete;

			EventHandler<T...>& operator=(const EventHandler<T...>& other);
			EventHandler<T...>& operator=(EventHandler<T...>&& other) = delete;

			bool operator==(const EventHandler<T...>& other) const;

			~EventHandler() = default;

			template<std::convertible_to<std::decay_t<T>>... Args>
			void operator()(Args&&... args)
			{
				m_Callback(std::forward<std::decay_t<T>>(args)...);
			}


		private:
			std::size_t m_ID;
			CallbackType m_Callback;

			static std::size_t s_NextHandlerID;
	};
}