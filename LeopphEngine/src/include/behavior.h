#pragma once


#include "leopphapi.h"
#include "component.h"
#include <set>


namespace leopph
{
	// Special component with updates called per frame. Users subclass this create game logic
	class Behavior : public Component
	{
	public:
		LEOPPHAPI static void UpdateAll();

		LEOPPHAPI Behavior(leopph::Object& object);
		LEOPPHAPI virtual ~Behavior() = 0;


		// THIS IS CALLED EVERY FRAME FOR ALL BEHAVIORS
		LEOPPHAPI virtual void OnFrameUpdate() {}

	private:
		static std::set<Behavior*> s_Behaviors;
	};
}