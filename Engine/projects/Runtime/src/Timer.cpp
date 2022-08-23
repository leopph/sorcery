#include "Timer.hpp"


namespace leopph
{
	f32 Timer::get_last_time() const
	{
		return mLastFrameTime;
	}



	f32 Timer::get_full_time() const
	{
		return mFullTime;
	}



	void Timer::init()
	{
		mLastFrameTime = 0;
		mFullTime = 0;
		mLastMeasurement = std::chrono::steady_clock::now();
	}



	void Timer::tick()
	{
		auto const now = std::chrono::steady_clock::now();
		mLastFrameTime = std::chrono::duration_cast<std::chrono::duration<f32, std::chrono::seconds::period>>(now - mLastMeasurement).count();
		mFullTime += mLastFrameTime;
		mLastMeasurement = now;
	}
}
