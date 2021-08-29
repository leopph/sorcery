#pragma once

#include "Event.hpp"

#include <cstddef>
#include <functional>
#include <type_traits>


namespace leopph
{
	class EventHandler
	{
	public:
		using ParameterType = const std::decay_t<Event>&;
		using CallbackType = void(ParameterType);

		explicit EventHandler(std::function<CallbackType> callback);

		EventHandler(const EventHandler& other) = default;
		EventHandler(EventHandler&& other) = default;
		EventHandler& operator=(const EventHandler& other) = default;
		EventHandler& operator=(EventHandler&& other) = default;
		~EventHandler() = default;

		void operator()(ParameterType event) const;

		bool operator==(const EventHandler& other) const;


	private:
		std::function<CallbackType> m_Callback;
		std::size_t m_Id;

		static std::size_t s_NextId;
	};
}