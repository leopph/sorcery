#include "timekeeping.h"

namespace leopph
{
	Time::TimePoint Time::s_LastFrameCompletionTime{ Clock::now() };
	Time::Seconds Time::s_LastFrameDeltaTime{};

	void Time::OnFrameComplete()
	{
		TimePoint currentTime{ Clock::now() };
		s_LastFrameDeltaTime = currentTime - s_LastFrameCompletionTime;
		s_LastFrameCompletionTime = currentTime;
	}

	float Time::DeltaTime()
	{
		return s_LastFrameDeltaTime.count();
	}
}