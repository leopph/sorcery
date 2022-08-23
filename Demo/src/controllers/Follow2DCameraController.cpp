#include "Follow2DCameraController.hpp"

using leopph::Camera;
using leopph::Vector2;


namespace demo
{
	Follow2DCameraController::Follow2DCameraController(leopph::Entity* target, Vector2 targetOffsetFromCenter) :
		mTraget{target},
		mOffset{targetOffsetFromCenter}
	{ }



	void Follow2DCameraController::on_frame_update()
	{
		Vector2 const targetPosXy{mTraget->get_position()[0], mTraget->get_position()[1]};
		get_owner()->set_position(leopph::Vector3{targetPosXy - mOffset, get_owner()->get_position()[2]});
	}
}
