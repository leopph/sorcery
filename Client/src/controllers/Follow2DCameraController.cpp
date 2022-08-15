#include "Follow2DCameraController.hpp"

using leopph::Camera;
using leopph::Transform;
using leopph::Vector2;
using leopph::ComponentPtr;


namespace demo
{
	Follow2DCameraController::Follow2DCameraController(ComponentPtr<Camera const> const& camera, Transform& target, Vector2 targetOffsetFromCenter) :
		m_CamTransform{&camera->Owner()->get_transform()},
		m_Target{&target},
		m_Offset{targetOffsetFromCenter}
	{ }


	void Follow2DCameraController::on_frame_update()
	{
		Vector2 const targetPosXy{m_Target->get_position()[0], m_Target->get_position()[1]};
		m_CamTransform->set_position(leopph::Vector3{targetPosXy - m_Offset, m_CamTransform->get_position()[2]});
	}
}
