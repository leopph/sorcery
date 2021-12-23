#include "leopphtime.h"
#include "timer.h"

namespace leopph
{
	float Time::DeltaTime()
	{
		return internal::Timer::DeltaTime();
	}

	float Time::FullTime()
	{
		return internal::Timer::FullTime();
	}
}