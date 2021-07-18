#pragma once

#include "leopph.h"

class FPSCounter final : public leopph::Behavior
{
public:
	FPSCounter();
	void Init() override;
	void OnFrameUpdate() override;

private:
	const float m_PollInterval;
	float m_DeltaTime = 0.0f;
};