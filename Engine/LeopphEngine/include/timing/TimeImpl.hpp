#pragma once

#include "EventReceiver.hpp"
#include "FrameCompleteEvent.hpp"

#include <chrono>


namespace leopph::internal
{
	// Internal-only class to measure time-related information around LeopphEngine.
	class TimeImpl final : public EventReceiver<FrameCompleteEvent>
	{
		public:
			[[nodiscard]] static TimeImpl& Instance();

			[[nodiscard]] float DeltaTime() const;
			[[nodiscard]] float FullTime() const;

			TimeImpl(TimeImpl const& other) = delete;
			TimeImpl& operator=(TimeImpl const& other) = delete;

			TimeImpl(TimeImpl&& other) noexcept = delete;
			TimeImpl& operator=(TimeImpl&& other) noexcept = delete;

		private:
			TimeImpl() = default;
			~TimeImpl() override = default;

			// Updates the timing information on frame completion.
			void OnEventReceived(EventParamType) override;

			using Clock = std::chrono::high_resolution_clock;
			using Seconds = std::chrono::duration<float>;
			using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

			TimePoint m_LastFrameCompletionTime{Clock::now()};
			Seconds m_LastFrameDeltaTime{};
			Seconds m_FullTime{};

			static TimeImpl s_Instance;
	};
}
