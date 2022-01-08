#include "FrameRateAnalyzer.hpp"

#include <iomanip>
#include <ios>
#include <iostream>
#include <numeric>


FrameRateAnalyzer::FrameRateAnalyzer(leopph::Entity* const entity, const float pollInterval, const unsigned maxDataSets) :
	Behavior{entity},
	m_MaxNumDataSets{maxDataSets},
	m_PollInterval{pollInterval},
	m_DeltaTime{0}
{}


auto FrameRateAnalyzer::OnFrameUpdate() -> void
{
	const auto delta{leopph::Time::DeltaTime()};
	m_DeltaTime += delta;

	if (m_DeltaTime >= m_PollInterval)
	{
		const FrameData frameData{.RatePerSec = 1.f / delta, .TimeInMs = delta * 1000};

		if (frameData.RatePerSec > m_Max.RatePerSec)
		{
			m_Max = frameData;
		}

		if (frameData.RatePerSec < m_Min.RatePerSec)
		{
			m_Min = frameData;
		}

		if (m_LastNData.size() > m_MaxNumDataSets)
		{
			m_LastNData.pop_front();
		}

		m_LastNData.push_back(frameData);

		const auto avgFps = std::accumulate(m_LastNData.begin(), m_LastNData.end(), 0.f, [](const auto sum, const auto elem)
		{
			return sum + elem.RatePerSec;
		}) / static_cast<float>(m_LastNData.size());

		const auto avgFrameTime{1000.f / avgFps};

		std::cout << std::fixed << std::setprecision(2)
			<< "##########" << std::endl
			<< "Min: " << m_Min.RatePerSec << " FPS, " << m_Min.TimeInMs << " ms" << std::endl
			<< "Avg: " << avgFps << " FPS, " << avgFrameTime << " ms" << std::endl
			<< "Max: " << m_Max.RatePerSec << " FPS, " << m_Max.TimeInMs << " ms" << std::endl
			<< "##########" << std::endl << std::endl;

		m_DeltaTime = 0;
	}
}
