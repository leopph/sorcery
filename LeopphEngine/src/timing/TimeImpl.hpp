#pragma once

#include "../events/FrameEndEvent.hpp"
#include "../events/handling/EventReceiver.hpp"

#include <chrono>


namespace leopph::internal
{
	// Internal-only class to measure time-related information around LeopphEngine.
	class TimeImpl final : public EventReceiver<FrameEndedEvent>
	{
		public:
			[[nodiscard]] static auto Instance() -> TimeImpl&;

			[[nodiscard]] auto DeltaTime() const -> float;
			[[nodiscard]] auto FullTime() const -> float;

			TimeImpl(const TimeImpl& other) = delete;
			auto operator=(const TimeImpl& other) -> TimeImpl& = delete;

			TimeImpl(TimeImpl&& other) noexcept = delete;
			auto operator=(TimeImpl&& other) noexcept -> TimeImpl& = delete;

		private:
			TimeImpl() = default;
			~TimeImpl() override = default;

			// Updates the timing information on frame completion.
			auto OnEventReceived(EventParamType) -> void override;

			using Clock = std::chrono::high_resolution_clock;
			using Seconds = std::chrono::duration<float>;
			using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

			TimePoint m_LastFrameCompletionTime{Clock::now()};
			Seconds m_LastFrameDeltaTime{};
			Seconds m_FullTime{};

			static TimeImpl s_Instance;
	};
}
