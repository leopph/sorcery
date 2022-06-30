#include "Time.hpp"

#include "timing/TimeImpl.hpp"


namespace leopph::time
{
	auto DeltaTime() -> float
	{
		return internal::TimeImpl::Instance().DeltaTime();
	}


	auto FullTime() -> float
	{
		return internal::TimeImpl::Instance().FullTime();
	}
}
