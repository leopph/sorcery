#pragma once


namespace leopph
{
	class Event
	{
	public:
		Event() = default;
		Event(const Event& other) = default;
		Event(Event&& other) = default;
		Event& operator=(const Event& other) = default;
		Event& operator=(Event&& other) = default;
		virtual ~Event() = 0;
	};
}
