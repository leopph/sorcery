#pragma once

#include <Leopph.hpp>


namespace demo
{
	class Follow2DCameraController final : public leopph::BehaviorNode
	{
		public:
			explicit Follow2DCameraController(leopph::CameraNode* camera, Node* target, leopph::Vector2 targetOffsetFromCenter);

			void on_frame_update() override;

		private:
			leopph::CameraNode* mCamera;
			Node* mTarget;
			leopph::Vector2 mOffset;
	};
}
