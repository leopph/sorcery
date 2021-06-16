#include "leopphtime.h"
#include "timer.h"

namespace leopph
{
	float Time::DeltaTime()
	{
		return impl::Timer::DeltaTime();
	}

	float Time::FullTime()
	{
		return impl::Timer::FullTime();
	}
}