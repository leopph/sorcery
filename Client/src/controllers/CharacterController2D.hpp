#pragma once

#include <Leopph.hpp>


namespace demo
{
	class CharacterController2D final : public leopph::Behavior
	{
		public:
			CharacterController2D(float speed, float runMult, float walkMult);

			void on_frame_update() override;

		private:
			float mSpeed;
			float mRunMult;
			float mWalkMult;
	};
}
