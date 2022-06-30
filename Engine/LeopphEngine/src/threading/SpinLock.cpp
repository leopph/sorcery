#include "threading/SpinLock.hpp"

#include <intrin.h>


namespace leopph::internal
{
	auto SpinLock::Lock() noexcept -> void
	{
		// Poll the lock while it is locked
		while (m_Lock.test_and_set(std::memory_order_acquire))
		{
			// PAUSE instruction to slow cpu down while spinning
			_mm_pause();
		}
	}


	auto SpinLock::Unlock() noexcept -> void
	{
		m_Lock.clear(std::memory_order_release);
	}
}
