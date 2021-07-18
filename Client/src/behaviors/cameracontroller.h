#pragma once

#include "leopph.h"

class CameraController final : public leopph::Behavior
{
public:
	CameraController();
	void Init() override;
	void OnFrameUpdate() override;

private:
	const float m_Speed;
	const float m_Sens;
	float lastX;
	float lastY;
};