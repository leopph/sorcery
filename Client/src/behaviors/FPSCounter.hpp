#pragma once

#include "leopph.h"

class FPSCounter final : public leopph::Behavior
{
public:
	explicit FPSCounter(leopph::Object& owner);
	void OnFrameUpdate() override;

private:
	const float m_PollInterval;
	float m_DeltaTime = 0.0f;
};