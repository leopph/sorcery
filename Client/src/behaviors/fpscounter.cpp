#include "FPSCounter.hpp"

#include <iostream>

FPSCounter::FPSCounter(leopph::Object& owner) :
	Behavior{ owner }, m_PollInterval{ 0.5f }, m_DeltaTime{ 0.0f }
{}

void FPSCounter::OnFrameUpdate()
{
	m_DeltaTime += leopph::Time::DeltaTime();

	if (m_DeltaTime >= m_PollInterval)
	{
		m_DeltaTime = 0.0f;
		std::cout << "FPS: " << 1 / leopph::Time::DeltaTime() << std::endl << "Frametime: " << leopph::Time::DeltaTime() * 1000 << " ms" << std::endl << std::endl;
	}
}