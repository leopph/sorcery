#include "DisplayResolutionChangedEvent.hpp"


namespace leopph::impl
{
	DisplayResolutionChangedEvent::DisplayResolutionChangedEvent(const Vector2& newRes) :
		newResolution{ m_Res }, m_Res { newRes }
	{}

}