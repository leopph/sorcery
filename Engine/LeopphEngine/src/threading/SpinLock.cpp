#include "SpinLock.hpp"

#include <intrin.h>


namespace leopph::internal
{
	void SpinLock::Lock() noexcept
	{
		// Poll the lock while it is locked
		while (m_Lock.test_and_set(std::memory_order_acquire))
		{
			// PAUSE instruction to slow cpu down while spinning
			_mm_pause();
		}
	}


	void SpinLock::Unlock() noexcept
	{
		m_Lock.clear(std::memory_order_release);
	}
}
