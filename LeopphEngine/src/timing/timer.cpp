#include "timer.h"


namespace leopph::internal
{
	Timer::TimePoint Timer::s_LastFrameCompletionTime{};
	Timer::Seconds Timer::s_LastFrameDeltaTime{};
	Timer::Seconds Timer::s_FullTime{};

	auto Timer::Init() -> void
	{
		s_LastFrameCompletionTime = Clock::now();
	}

	auto Timer::OnFrameComplete() -> void
	{
		auto currentTime{Clock::now()};
		s_LastFrameDeltaTime = currentTime - s_LastFrameCompletionTime;
		s_LastFrameCompletionTime = currentTime;
		s_FullTime += s_LastFrameDeltaTime;
	}

	auto Timer::DeltaTime() -> float
	{
		return s_LastFrameDeltaTime.count();
	}

	auto Timer::FullTime() -> float
	{
		return s_FullTime.count();
	}
}
