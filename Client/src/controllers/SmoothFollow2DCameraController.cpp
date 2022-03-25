#include "SmoothFollow2DCameraController.hpp"

using leopph::Vector2;


namespace demo
{
	SmoothFollow2DCameraController::SmoothFollow2DCameraController(leopph::Camera const* camera, std::shared_ptr<leopph::Transform> target, Vector2 const targetOffsetFromCenter, float const followSpeed) :
		m_Target{std::move(target)},
		m_CamTransform{camera->Entity()->Transform()},
		m_Offset{targetOffsetFromCenter},
		m_Speed{followSpeed}
	{ }


	auto SmoothFollow2DCameraController::OnFrameUpdate() -> void
	{
		auto static constexpr epsilon = 0.1f;

		if (Vector2 camPos{m_CamTransform->Position()}, targetPos{m_Target->Position()}; Vector2::Distance(camPos, targetPos - m_Offset) > epsilon)
		{
			auto const dir = (targetPos - m_Offset - camPos).Normalized() * leopph::time::DeltaTime() * m_Speed;
			m_CamTransform->Translate(dir[0], dir[1], 0, leopph::Space::World);
		}
	}
}
