#pragma once

#include "leopph.h"

class Rotate final : public leopph::Behavior
{
public:
	explicit Rotate(leopph::Object& owner);
	void OnFrameUpdate() override;

private:
	const leopph::Vector3 m_Axis;
	const float m_Angle;
};