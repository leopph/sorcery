#pragma once

#include <Leopph.hpp>


namespace demo
{
	class SmoothFollow2DCameraController final : public leopph::Behavior
	{
		public:
			SmoothFollow2DCameraController(leopph::ComponentPtr<leopph::Camera const> const& camera, leopph::ComponentPtr<leopph::Transform> target, leopph::Vector2 targetOffsetFromCenter, float followSpeed);

			auto OnFrameUpdate() -> void override;

		private:
			leopph::ComponentPtr<leopph::Transform> m_Target;
			leopph::ComponentPtr<leopph::Transform> m_CamTransform;
			leopph::Vector2 m_Offset;
			float m_Speed;
	};
}