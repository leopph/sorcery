#pragma once

#include <Leopph.hpp>


namespace demo
{
	class Follow2DCameraController final : public leopph::Behavior
	{
		public:
			explicit Follow2DCameraController(leopph::Entity* target, leopph::Vector2 targetOffsetFromCenter);

			void on_frame_update() override;

		private:
			leopph::Entity* mTraget;
			leopph::Vector2 mOffset;
	};
}
