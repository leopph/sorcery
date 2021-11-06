#pragma once

#include "Event.hpp"
#include "../math/Vector.hpp"


namespace leopph::impl
{
	class ScreenResolutionEvent final : public Event
	{
	public:
		explicit ScreenResolutionEvent(const Vector2& newRes);

		const Vector2& newResolution;


	private:
		Vector2 m_Res;
	};
}
