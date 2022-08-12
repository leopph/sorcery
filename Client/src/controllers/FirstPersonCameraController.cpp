#include "FirstPersonCameraController.hpp"

using leopph::Camera;
using leopph::ComponentPtr;
using leopph::Input;
using leopph::KeyCode;


namespace demo
{
	FirstPersonCameraController::FirstPersonCameraController(ComponentPtr<Camera const> const& camera, float const speed, float const sens, float const runMult, float const walkMult) :
		FirstPersonCameraController{camera, speed, sens, runMult, walkMult, Input::GetMousePosition()}
	{ }


	FirstPersonCameraController::FirstPersonCameraController(ComponentPtr<Camera const> const& camera, float const speed, float const sens, float const runMult, float const walkMult, std::tuple<float, float> const& mousePos) :
		m_CamTransform{&camera->Owner()->get_transform()},
		m_Speed{speed},
		m_Sens{sens},
		m_RunMult{runMult},
		m_WalkMult{walkMult},
		m_LastX{std::get<0>(mousePos)},
		m_LastY{std::get<1>(mousePos)}
	{ }


	void FirstPersonCameraController::OnFrameUpdate()
	{
		leopph::Vector3 movementVector;

		if (Input::GetKey(KeyCode::W))
		{
			movementVector += m_CamTransform->get_forward_axis();
		}

		if (Input::GetKey(KeyCode::S))
		{
			movementVector -= m_CamTransform->get_forward_axis();
		}

		if (Input::GetKey(KeyCode::D))
		{
			movementVector += m_CamTransform->get_right_axis();
		}

		if (Input::GetKey(KeyCode::A))
		{
			movementVector -= m_CamTransform->get_right_axis();
		}

		if (Input::GetKey(KeyCode::Space))
		{
			movementVector += leopph::Vector3::up();
		}

		if (Input::GetKey(KeyCode::LeftControl))
		{
			movementVector += leopph::Vector3::down();
		}

		movementVector.normalize();

		if (Input::GetKey(KeyCode::LeftShift))
		{
			movementVector *= m_RunMult;
		}

		if (Input::GetKey(KeyCode::LeftAlt))
		{
			movementVector *= m_WalkMult;
		}

		m_CamTransform->translate(movementVector * m_Speed * leopph::time::DeltaTime(), leopph::Space::World);

		auto const [posX, posY] = Input::GetMousePosition();
		auto const diffX = posX - m_LastX;
		auto const diffY = posY - m_LastY;

		short coefficient = 1;
		if (leopph::Vector3::dot(m_CamTransform->get_up_axis(), leopph::Vector3::up()) < 0)
		{
			coefficient = -1;
		}

		m_CamTransform->rotate(coefficient * leopph::Vector3::up(), diffX * m_Sens, leopph::Space::World);
		m_CamTransform->rotate(leopph::Vector3::right(), diffY * m_Sens, leopph::Space::Local);

		m_LastX = posX;
		m_LastY = posY;
	}
}
