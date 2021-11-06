#include "ScreenResolutionEvent.hpp"


namespace leopph::impl
{
	ScreenResolutionEvent::ScreenResolutionEvent(const Vector2& newRes) :
		newResolution{ m_Res }, m_Res { newRes }
	{}

}