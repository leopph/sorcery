#include "SmoothFollow2DCameraController.hpp"

using leopph::Vector2;
using leopph::ComponentPtr;
using leopph::Transform;
using leopph::Camera;


namespace demo
{
	SmoothFollow2DCameraController::SmoothFollow2DCameraController(ComponentPtr<Camera const> const& camera, ComponentPtr<Transform> target, Vector2 const targetOffsetFromCenter, float const followSpeed) :
		m_Target{std::move(target)},
		m_CamTransform{camera->Owner()->Transform()},
		m_Offset{targetOffsetFromCenter},
		m_Speed{followSpeed}
	{ }


	auto SmoothFollow2DCameraController::OnFrameUpdate() -> void
	{
		if (!m_Target || !m_Target->Attached() || !m_CamTransform || !m_CamTransform->Attached())
		{
			return;
		}

		auto static constexpr epsilon = 0.1f;

		if (Vector2 camPos{m_CamTransform->Position()}, targetPos{m_Target->Position()}; Vector2::Distance(camPos, targetPos - m_Offset) > epsilon)
		{
			auto const dir = (targetPos - m_Offset - camPos).Normalized() * leopph::time::DeltaTime() * m_Speed;
			m_CamTransform->Translate(dir[0], dir[1], 0, leopph::Space::World);
		}
	}
}
