#pragma once

#include <atomic>


namespace leopph::internal
{
	class SpinLock
	{
		public:
			auto Lock() noexcept -> void;

			auto Unlock() noexcept -> void;

			SpinLock() = default;

			SpinLock(const SpinLock& other) = delete;
			auto operator=(const SpinLock& other) = delete;

			SpinLock(SpinLock&& other) noexcept = delete;
			auto operator=(SpinLock&& other) noexcept = delete;

			~SpinLock() noexcept = default;

		private:
			// True for locked state
			std::atomic_flag m_Lock;
	};
}