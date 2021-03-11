#include "timekeeping.h"

namespace leopph
{
	Time::TimePoint Time::s_LastFrameCompletionTime{};
	Time::Seconds Time::s_LastFrameDeltaTime{};
	Time::Seconds Time::s_FullTime{};

	void Time::Init()
	{
		s_LastFrameCompletionTime = Clock::now();
	}

	void Time::OnFrameComplete()
	{
		TimePoint currentTime{ Clock::now() };
		s_LastFrameDeltaTime = currentTime - s_LastFrameCompletionTime;
		s_LastFrameCompletionTime = currentTime;
		s_FullTime += s_LastFrameDeltaTime;
	}

	float Time::DeltaTime()
	{
		return s_LastFrameDeltaTime.count();
	}

	float Time::FullTime()
	{
		return s_FullTime.count();
	}
}