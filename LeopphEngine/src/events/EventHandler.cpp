#include "EventHandler.hpp"


namespace leopph
{
	template<class... T>
	std::size_t EventHandler<T...>::s_NextHandlerID{};


	template<class ... T>
	EventHandler<T...>::EventHandler(CallbackType function) :
		m_ID{ s_NextHandlerID++ }, m_Callback{ std::move(function) }
	{
		
	}


	template<class ... T>
	EventHandler<T...>::EventHandler(const EventHandler<T...>& other) :
		m_ID{ other.m_ID }, m_Callback{ other.m_Callback }
	{
		
	}


	template<class ... T>
	EventHandler<T...>& EventHandler<T...>::operator=(const EventHandler& other)
	{
		m_ID = other.m_ID;
		m_Callback = other.m_Callback;
		return *this;
	}


	template<class ... T>
	bool EventHandler<T...>::operator==(const EventHandler<T...>& other) const
	{
		return m_ID == other.m_ID;
	}

}