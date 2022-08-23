#pragma once

#include "Leopph.hpp"

#include <limits>
#include <list>


class FrameRateAnalyzer final : public leopph::Behavior
{
	public:
		explicit FrameRateAnalyzer(float pollInterval, unsigned maxDataSets);
		void on_frame_update() override;

	private:
		struct FrameData
		{
			float RatePerSec;
			float TimeInMs;
		};


		std::list<FrameData> m_LastNData;
		FrameData m_Min{.RatePerSec = std::numeric_limits<float>::max(), .TimeInMs = 0};
		FrameData m_Max{.RatePerSec = std::numeric_limits<float>::min(), .TimeInMs = 0};
		unsigned m_MaxNumDataSets;
		float m_PollInterval;
		float m_DeltaTime;

		constexpr static char const* DATA_SEPARATOR{"################"};
		constexpr static char const* FPS_POSTFIX{" FPS, "};
		constexpr static char const* FRAMETIME_POSTFIX{" ms"};
};
