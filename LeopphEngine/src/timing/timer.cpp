#include "timer.h"

namespace leopph::internal
{
	Timer::TimePoint Timer::s_LastFrameCompletionTime{};
	Timer::Seconds Timer::s_LastFrameDeltaTime{};
	Timer::Seconds Timer::s_FullTime{};

	void Timer::Init()
	{
		s_LastFrameCompletionTime = Clock::now();
	}

	void Timer::OnFrameComplete()
	{
		TimePoint currentTime{ Clock::now() };
		s_LastFrameDeltaTime = currentTime - s_LastFrameCompletionTime;
		s_LastFrameCompletionTime = currentTime;
		s_FullTime += s_LastFrameDeltaTime;
	}

	float Timer::DeltaTime()
	{
		return s_LastFrameDeltaTime.count();
	}

	float Timer::FullTime()
	{
		return s_FullTime.count();
	}
}