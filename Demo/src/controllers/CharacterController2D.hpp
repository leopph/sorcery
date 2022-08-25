#pragma once

#include <Leopph.hpp>


namespace demo
{
	class CharacterController2D final : public leopph::BehaviorNode
	{
		public:
			CharacterController2D(leopph::CameraNode* camera, float speed, float runMult, float walkMult);

			void on_frame_update() override;

		private:
			leopph::CameraNode* mCamera;
			float mSpeed;
			float mRunMult;
			float mWalkMult;
	};
}
