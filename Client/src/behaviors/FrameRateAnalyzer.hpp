#pragma once

#include "Leopph.hpp"

#include <limits>
#include <list>


class FrameRateAnalyzer final : public leopph::Behavior
{
	public:
		explicit FrameRateAnalyzer(leopph::Entity* entity, float pollInterval, unsigned maxDataSets);
		auto OnFrameUpdate() -> void override;

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

		constexpr static const char* DATA_SEPARATOR{"################"};
		constexpr static const char* FPS_POSTFIX{" FPS, "};
		constexpr static const char* FRAMETIME_POSTFIX{" ms"};
};
