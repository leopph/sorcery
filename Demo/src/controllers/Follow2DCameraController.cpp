#include "Follow2DCameraController.hpp"

using leopph::CameraNode;
using leopph::Vector2;


namespace demo
{
	Follow2DCameraController::Follow2DCameraController(leopph::CameraNode* camera, Node* target, Vector2 targetOffsetFromCenter) :
		mCamera{camera},
		mTarget{target},
		mOffset{targetOffsetFromCenter}
	{ }



	void Follow2DCameraController::on_frame_update()
	{
		Vector2 const targetPosXy{mTarget->get_position()[0], mTarget->get_position()[1]};
		mCamera->set_position(leopph::Vector3{targetPosXy - mOffset, mCamera->get_position()[2]});
	}
}
