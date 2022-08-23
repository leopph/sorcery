#pragma once

#include <Leopph.hpp>
#include <span>
#include <vector>


namespace demo
{
	class Parallaxer final : public leopph::Behavior
	{
		public:
			struct Layer
			{
				// The layer will be moved with speedMult * the camera's speed
				float speedMult;
				leopph::Entity* entity;
			};


			explicit Parallaxer(leopph::Camera* camera, std::span<Layer> layers = {});

			void on_frame_update() override;

		private:
			std::vector<Layer> mLayers;
			leopph::Camera* mTargetCam;
			float mPrevCamPosX;
	};
}
