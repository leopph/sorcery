#include "EventHandler.hpp"


namespace leopph
{
	std::size_t EventHandler::s_NextId{};


	EventHandler::EventHandler(std::function<CallbackType> callback) :
		m_Callback{ std::move(callback) }, m_Id{ s_NextId++ }
	{}


	void EventHandler::operator()(ParameterType event) const
	{
		m_Callback(event);
	}


	bool EventHandler::operator==(const EventHandler& other) const
	{
		return m_Id == other.m_Id;
	}
}
