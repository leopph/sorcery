#include "FrameRateAnalyzer.hpp"

#include <iomanip>
#include <ios>
#include <iostream>
#include <numeric>


FrameRateAnalyzer::FrameRateAnalyzer(float const pollInterval, unsigned const maxDataSets) :
	m_MaxNumDataSets{maxDataSets},
	m_PollInterval{pollInterval},
	m_DeltaTime{0}
{}


void FrameRateAnalyzer::on_frame_update()
{
	auto const delta{leopph::time::DeltaTime()};
	m_DeltaTime += delta;

	if (m_DeltaTime >= m_PollInterval)
	{
		FrameData const curFrameData{.RatePerSec = 1.f / delta, .TimeInMs = delta * 1000};

		if (curFrameData.RatePerSec > m_Max.RatePerSec)
		{
			m_Max = curFrameData;
		}

		if (curFrameData.RatePerSec < m_Min.RatePerSec)
		{
			m_Min = curFrameData;
		}

		if (m_LastNData.size() > m_MaxNumDataSets)
		{
			m_LastNData.pop_front();
		}

		m_LastNData.push_back(curFrameData);

		auto const avgFps = std::accumulate(m_LastNData.begin(), m_LastNData.end(), 0.f, [](auto const sum, auto const elem)
		{
			return sum + elem.RatePerSec;
		}) / static_cast<float>(m_LastNData.size());

		auto const avgFrameTime{1000.f / avgFps};

		std::cout << std::fixed << std::setprecision(2)
			<< DATA_SEPARATOR << std::endl
			<< "Min:     " << m_Min.RatePerSec << FPS_POSTFIX << m_Min.TimeInMs << FRAMETIME_POSTFIX << std::endl
			<< "Avg:     " << avgFps << FPS_POSTFIX << avgFrameTime << FRAMETIME_POSTFIX << std::endl
			<< "Max:     " << m_Max.RatePerSec << FPS_POSTFIX << m_Max.TimeInMs << FRAMETIME_POSTFIX << std::endl
			<< "Current: " << curFrameData.RatePerSec << FPS_POSTFIX << curFrameData.TimeInMs << FRAMETIME_POSTFIX << std::endl
			<< DATA_SEPARATOR << std::endl << std::endl;

		m_DeltaTime = 0;
	}
}
