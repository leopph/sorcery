#pragma once

#include <Leopph.hpp>


namespace demo
{
	class Follow2DCameraController final : public leopph::Behavior
	{
		public:
			explicit Follow2DCameraController(leopph::Camera const* camera, leopph::Transform* target, leopph::Vector2 targetOffsetFromCenter);

			auto OnFrameUpdate() -> void override;

		private:
			leopph::Transform* m_CamTransform;
			leopph::Transform* m_Target;
			leopph::Vector2 m_Offset;
	};
}
