#pragma once

#include "Leopph.hpp"

class FPSCounter final : public leopph::Behavior
{
public:
	explicit FPSCounter(leopph::Entity* entity);
	void OnFrameUpdate() override;

private:
	const float m_PollInterval;
	float m_DeltaTime = 0.0f;
};