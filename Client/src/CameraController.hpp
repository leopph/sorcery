#pragma once

#include "Leopph.hpp"


class CameraController final : public leopph::Behavior
{
	public:
		explicit CameraController();
		auto OnFrameUpdate() -> void override;

	private:
		const float m_Speed{2.0f};
		const float m_Sens{0.1f};
		float m_LastX{};
		float m_LastY{};
		constexpr static float RUN_MULT{3.f};
};
