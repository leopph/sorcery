#pragma once

#include "../math/Vector.hpp"
#include "Event.hpp"


namespace leopph::impl
{
	class DisplayResolutionChangedEvent final : public Event
	{
	public:
		explicit DisplayResolutionChangedEvent(const Vector2& newRes);

		const Vector2& newResolution;


	private:
		Vector2 m_Res;
	};
}
