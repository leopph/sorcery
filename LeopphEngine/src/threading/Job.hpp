#pragma once

#include <cstddef>
#include <functional>
#include <future>


namespace leopph::internal
{
	class Job
	{
		public:
			using JobFunc = std::function<void()>;
			using FutureType = std::future<void>;

			explicit Job(JobFunc func = {});

			Job(const Job& other) = delete;
			auto operator=(const Job& other) -> Job& = delete;

			Job(Job&& other) noexcept = default;
			auto operator=(Job&& other) noexcept -> Job& = default;

			~Job() = default;

			auto operator()() -> void;

			// This can only be called once.
			// Since LeopphEngine calls it internally, make sure you do not.
			[[nodiscard]] auto Future() -> FutureType;


		private:
			std::size_t m_Id;
			JobFunc m_Func;
			std::promise<void> m_Promise;

			[[nodiscard]] static auto GenId() noexcept -> std::size_t;
	};
}
