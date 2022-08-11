#pragma once

namespace leopph
{
	// Event is the base class for all of the objects that can circulate in the Event System.
	// Subclass this to add your own data and broadcast it.
	class Event
	{
		protected:
			Event() = default;

		public:
			Event(Event const& other) = default;
			Event& operator=(Event const& other) = default;

			Event(Event&& other) noexcept = default;
			Event& operator=(Event&& other) noexcept = default;

			virtual ~Event() = 0;
	};
}
