#pragma once

#include "Event.hpp"
#include "../math/Vector.hpp"


namespace leopph::internal
{
	class WindowEvent final : public Event
	{
		public:
			explicit WindowEvent(const Vector2& newScreenRes, float newResMult, bool vsync, bool fullscreen);

			const Vector2 Resolution;
			const float RenderMultiplier;
			const bool Vsync;
			const bool Fullscreen;
	};
}
