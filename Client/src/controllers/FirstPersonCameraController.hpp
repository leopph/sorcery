#pragma once

#include "Leopph.hpp"

#include <tuple>


namespace demo
{
	class FirstPersonCameraController final : public leopph::Behavior
	{
		public:
			explicit FirstPersonCameraController(leopph::ComponentPtr<leopph::Camera const> const& camera, float speed, float sens, float runMult, float walkMult);

		private:
			explicit FirstPersonCameraController(leopph::ComponentPtr<leopph::Camera const> const& camera, float speed, float sens, float runMult, float walkMult, std::tuple<float, float> const& mousePos);

		public:
			auto OnFrameUpdate() -> void override;

		private:
			leopph::ComponentPtr<leopph::Transform> m_CamTransform;
			float m_Speed;
			float m_Sens;
			float m_RunMult;
			float m_WalkMult;
			float m_LastX;
			float m_LastY;
	};
}
