#include "leopphtime.h"

#include "timer.h"


namespace leopph
{
	auto Time::DeltaTime() -> float
	{
		return internal::Timer::DeltaTime();
	}

	auto Time::FullTime() -> float
	{
		return internal::Timer::FullTime();
	}
}
