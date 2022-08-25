#pragma once

#include "Leopph.hpp"


namespace demo
{
	class FirstPersonCameraController final : public leopph::BehaviorNode
	{
		public:
			FirstPersonCameraController(leopph::CameraNode* camera, float speed, float sens, float runMult, float walkMult);

			void on_frame_update() override;

		private:
			leopph::CameraNode* mCamera;
			float mSpeed;
			float mMouseSens;
			float mRunMult;
			float mWalkMult;
			float mMouseX;
			float mMouseY;
	};
}
