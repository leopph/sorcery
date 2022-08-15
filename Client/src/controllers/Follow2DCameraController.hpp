#pragma once

#include <Leopph.hpp>


namespace demo
{
	class Follow2DCameraController final : public leopph::Behavior
	{
		public:
			explicit Follow2DCameraController(leopph::ComponentPtr<leopph::Camera const> const& camera, leopph::Transform& target, leopph::Vector2 targetOffsetFromCenter);

			void on_frame_update() override;

		private:
			leopph::Transform* m_CamTransform;
			leopph::Transform* m_Target;
			leopph::Vector2 m_Offset;
	};
}
