#pragma once

#include "Leopph.hpp"

#include <tuple>


namespace demo
{
	class FirstPersonCameraController final : public leopph::Behavior
	{
		public:
			explicit FirstPersonCameraController(leopph::Camera const* camera, float speed, float sens, float runMult, float walkMult);

		private:
			explicit FirstPersonCameraController(leopph::Camera const* camera, float speed, float sens, float runMult, float walkMult, std::tuple<float, float> const& mousePos);

		public:
			auto OnFrameUpdate() -> void override;

		private:
			std::shared_ptr<leopph::Transform> m_CamTransform;
			float m_Speed;
			float m_Sens;
			float m_RunMult;
			float m_WalkMult;
			float m_LastX;
			float m_LastY;
	};
}
