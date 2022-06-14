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


			// Labels define which threads the Job will be able to run on
			enum class Label
			{
				Render, // Only the render thread can run this
				Misc // General-purpose threads will process this
			};


			explicit Job(JobFunc func, Label label = Label::Misc);

			Job(const Job& other) = delete;
			auto operator=(const Job& other) -> Job& = delete;

			Job(Job&& other) noexcept = default;
			auto operator=(Job&& other) noexcept -> Job& = default;

			~Job() = default;

			auto operator()() -> void;

			// This can only be called once.
			// Since LeopphEngine calls it internally, make sure you do not.
			[[nodiscard]] auto Future() -> FutureType;

			[[nodiscard]] constexpr auto Id() const noexcept;

			[[nodiscard]] constexpr auto GetLabel() const noexcept;

		private:
			std::size_t m_Id;
			Label m_Label;
			JobFunc m_Func;
			std::promise<void> m_Promise;

			[[nodiscard]] static auto GenId() noexcept -> std::size_t;
	};


	constexpr auto Job::Id() const noexcept
	{
		return m_Id;
	}


	constexpr auto Job::GetLabel() const noexcept
	{
		return m_Label;
	}
}
