#pragma once

#include "Leopph.hpp"


namespace demo
{
	class FirstPersonCameraController final : public leopph::Behavior
	{
		public:
			FirstPersonCameraController(float speed, float sens, float runMult, float walkMult);

			void on_frame_update() override;

		private:
			float mSpeed;
			float mMouseSens;
			float mRunMult;
			float mWalkMult;
			float mMouseX;
			float mMouseY;
	};
}
