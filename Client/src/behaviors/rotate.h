#pragma once

#include "leopph.h"

class Rotate final : public leopph::Behavior
{
public:
	Rotate();
	void OnFrameUpdate() override;

private:
	const leopph::Vector3 m_Axis;
	const float m_Angle;
};