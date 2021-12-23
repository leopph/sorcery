#pragma once

#include "../api/LeopphApi.hpp"


namespace leopph
{
	/* Event is the base class for all of the objects that
	 * can circulate in the Event System. Subclass this to
	 * add your own data and broadcast it. */
	class Event
	{
		public:
			LEOPPHAPI Event() = default;
			LEOPPHAPI Event(const Event& other) = default;
			LEOPPHAPI Event(Event&& other) = default;
			LEOPPHAPI auto operator=(const Event& other) -> Event& = default;
			LEOPPHAPI auto operator=(Event&& other) -> Event& = default;
			LEOPPHAPI virtual ~Event() = 0;
	};
}
