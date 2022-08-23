#pragma once

#include <atomic>


namespace leopph::internal
{
	class SpinLock
	{
		public:
			void Lock() noexcept;

			void Unlock() noexcept;

			SpinLock() = default;

			SpinLock(SpinLock const& other) = delete;
			auto operator=(SpinLock const& other) = delete;

			SpinLock(SpinLock&& other) noexcept = delete;
			auto operator=(SpinLock&& other) noexcept = delete;

			~SpinLock() noexcept = default;

		private:
			// True for locked state
			std::atomic_flag m_Lock;
	};
}
