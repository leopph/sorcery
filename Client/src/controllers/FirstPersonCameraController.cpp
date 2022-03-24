#include "FirstPersonCameraController.hpp"


namespace demo
{
	FirstPersonCameraController::FirstPersonCameraController(leopph::Camera const* camera, float const speed, float const sens, float const runMult, float const walkMult) :
		FirstPersonCameraController{camera, speed, sens, runMult, walkMult, leopph::Input::GetMousePosition()}
	{ }


	FirstPersonCameraController::FirstPersonCameraController(leopph::Camera const* camera, float const speed, float const sens, float const runMult, float const walkMult, std::tuple<float, float> const& mousePos) :
		m_CamTransform{camera->Entity()->Transform()},
		m_Speed{speed},
		m_Sens{sens},
		m_RunMult{runMult},
		m_WalkMult{walkMult},
		m_LastX{std::get<0>(mousePos)},
		m_LastY{std::get<1>(mousePos)}
	{ }


	auto FirstPersonCameraController::OnFrameUpdate() -> void
	{
		leopph::Vector3 movementVector;

		if (leopph::Input::GetKey(leopph::KeyCode::W))
		{
			movementVector += m_CamTransform->Forward();
		}

		if (leopph::Input::GetKey(leopph::KeyCode::S))
		{
			movementVector -= m_CamTransform->Forward();
		}

		if (leopph::Input::GetKey(leopph::KeyCode::D))
		{
			movementVector += m_CamTransform->Right();
		}

		if (leopph::Input::GetKey(leopph::KeyCode::A))
		{
			movementVector -= m_CamTransform->Right();
		}

		if (leopph::Input::GetKey(leopph::KeyCode::Space))
		{
			movementVector += leopph::Vector3::Up();
		}

		if (leopph::Input::GetKey(leopph::KeyCode::LeftControl))
		{
			movementVector += leopph::Vector3::Down();
		}

		movementVector.Normalize();

		if (leopph::Input::GetKey(leopph::KeyCode::LeftShift))
		{
			movementVector *= m_RunMult;
		}

		if (leopph::Input::GetKey(leopph::KeyCode::LeftAlt))
		{
			movementVector *= m_WalkMult;
		}

		m_CamTransform->Translate(movementVector * m_Speed * leopph::time::DeltaTime(), leopph::Space::World);

		auto const [posX, posY] = leopph::Input::GetMousePosition();
		auto const diffX = posX - m_LastX;
		auto const diffY = posY - m_LastY;

		short coefficient = 1;
		if (leopph::Vector3::Dot(m_CamTransform->Up(), leopph::Vector3::Up()) < 0)
		{
			coefficient = -1;
		}

		m_CamTransform->Rotate(coefficient * leopph::Vector3::Up(), diffX * m_Sens, leopph::Space::World);
		m_CamTransform->Rotate(leopph::Vector3::Right(), diffY * m_Sens, leopph::Space::Local);

		m_LastX = posX;
		m_LastY = posY;
	}
}
