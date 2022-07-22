#include "Time.hpp"

#include "timing/TimeImpl.hpp"


namespace leopph::time
{
	float DeltaTime()
	{
		return internal::TimeImpl::Instance().DeltaTime();
	}


	float FullTime()
	{
		return internal::TimeImpl::Instance().FullTime();
	}
}
