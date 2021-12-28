#pragma once

namespace leopph
{
	// Event is the base class for all of the objects that can circulate in the Event System.
	// Subclass this to add your own data and broadcast it.
	class Event
	{
		public:
			virtual ~Event() = 0;

		protected:
			Event() = default;

			Event(const Event& other) = default;
			auto operator=(const Event& other) -> Event& = default;

			Event(Event&& other) noexcept = default;
			auto operator=(Event&& other) noexcept -> Event& = default;
	};
}
