#pragma once

#include <Leopph.hpp>


namespace demo
{
	class SmoothFollow2DCameraController final : public leopph::Behavior
	{
		public:
			SmoothFollow2DCameraController(leopph::Camera const* camera, std::shared_ptr<leopph::Transform> target, leopph::Vector2 targetOffsetFromCenter, float followSpeed);

			auto OnFrameUpdate() -> void override;

		private:
			std::shared_ptr<leopph::Transform> m_Target;
			std::shared_ptr<leopph::Transform> m_CamTransform;
			leopph::Vector2 m_Offset;
			float m_Speed;
	};
}
