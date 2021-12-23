#include "ScreenResolutionEvent.hpp"


namespace leopph::internal
{
	ScreenResolutionEvent::ScreenResolutionEvent(const Vector2& newScreenRes, const float newResMult) :
		NewResolution{newScreenRes},
		NewResolutionMultiplier{newResMult}
	{}
}
