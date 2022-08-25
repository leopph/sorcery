#pragma once

#include <Leopph.hpp>
#include <span>
#include <vector>


namespace demo
{
	class Parallaxer final : public leopph::BehaviorNode
	{
		public:
			struct Layer
			{
				// The layer will be moved with speedMult * the camera's speed
				float speedMult;
				leopph::Node* entity;
			};


			explicit Parallaxer(leopph::CameraNode* camera, std::span<Layer> layers = {});

			void on_frame_update() override;

		private:
			std::vector<Layer> mLayers;
			leopph::CameraNode* mTargetCam;
			float mPrevCamPosX;
	};
}
