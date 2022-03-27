#include "Follow2DCameraController.hpp"

using leopph::Camera;
using leopph::Transform;
using leopph::Vector2;
using leopph::ComponentPtr;


namespace demo
{
	Follow2DCameraController::Follow2DCameraController(ComponentPtr<Camera const> const& camera, ComponentPtr<Transform> target, Vector2 targetOffsetFromCenter) :
		m_CamTransform{camera->Owner()->Transform()},
		m_Target{std::move(target)},
		m_Offset{targetOffsetFromCenter}
	{ }


	auto Follow2DCameraController::OnFrameUpdate() -> void
	{
		Vector2 const targetPosXy{m_Target->Position()[0], m_Target->Position()[1]};
		m_CamTransform->Position(leopph::Vector3{targetPosXy - m_Offset, m_CamTransform->Position()[2]});
	}
}
