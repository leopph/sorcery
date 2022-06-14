#pragma once

#include "Job.hpp"

#include <thread>
#include <utility>


namespace leopph::internal
{
	// A special thread with a job label attached
	class Worker final : public std::thread
	{
		public:
			template<class Function, class... Args>
			Worker(Job::Label label, Function&& f, Args&&... args);

			Worker(const Worker& other) = delete;
			auto operator=(const Worker& other) = delete;

			Worker(Worker&& other) noexcept;
			auto operator=(Worker&& other) noexcept -> Worker&;

			// Destructor is not virtual, DO NOT DELETE WORKER THROUGH std::thread POINTER
			~Worker() = default;

			[[nodiscard]] constexpr auto Label() const noexcept;

		private:
			Job::Label m_Label;
	};


	template<class Function, class ... Args>
	Worker::Worker(const Job::Label label, Function&& f, Args&&... args) :
		std::thread{std::forward<Function>(f), std::forward<Args>(args)...},
		m_Label{label}
	{}


	constexpr auto Worker::Label() const noexcept
	{
		return m_Label;
	}
}
