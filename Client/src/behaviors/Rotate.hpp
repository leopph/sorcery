#pragma once

#include "Leopph.hpp"

class Rotate final : public leopph::Behavior
{
public:
	explicit Rotate(leopph::Entity& owner, const leopph::Vector3& axis, float anglePerSec, bool rotateLocally = false);
	void OnFrameUpdate() override;

private:
	const leopph::Vector3 m_Axis;
	const float m_Angle;
	const bool m_RotateLocally;
};