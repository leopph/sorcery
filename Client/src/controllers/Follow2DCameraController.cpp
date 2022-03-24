#include "Follow2DCameraController.hpp"


namespace demo
{
	Follow2DCameraController::Follow2DCameraController(leopph::Camera const* camera, leopph::Transform* target, leopph::Vector2 targetOffsetFromCenter) :
		m_CamTransform{camera->Entity()->Transform()},
		m_Target{target},
		m_Offset{targetOffsetFromCenter}
	{ }


	auto Follow2DCameraController::OnFrameUpdate() -> void
	{
		leopph::Vector2 const targetPosXy{m_Target->Position()[0], m_Target->Position()[1]};
		m_CamTransform->Position(leopph::Vector3{targetPosXy - m_Offset, m_CamTransform->Position()[2]});
	}
}
