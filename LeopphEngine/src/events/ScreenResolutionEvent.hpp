#pragma once

#include "Event.hpp"
#include "../math/Vector.hpp"


namespace leopph::internal
{
	class ScreenResolutionEvent final : public Event
	{
		public:
			explicit ScreenResolutionEvent(const Vector2& newScreenRes, float newResMult);

			const Vector2 NewResolution;
			const float NewResolutionMultiplier;
	};
}
