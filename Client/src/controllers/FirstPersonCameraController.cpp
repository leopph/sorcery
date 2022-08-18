#include "FirstPersonCameraController.hpp"

using leopph::Camera;
using leopph::Input;
using leopph::KeyCode;


namespace demo
{
	FirstPersonCameraController::FirstPersonCameraController(float const speed, float const sens, float const runMult, float const walkMult) :
		mSpeed{speed},
		mMouseSens{sens},
		mRunMult{runMult},
		mWalkMult{walkMult},
		mMouseX{Input::GetMousePosition().first},
		mMouseY{Input::GetMousePosition().second}
	{ }



	void FirstPersonCameraController::on_frame_update()
	{
		leopph::Vector3 movementVector;

		if (Input::GetKey(KeyCode::W))
		{
			movementVector += get_owner()->get_forward_axis();
		}

		if (Input::GetKey(KeyCode::S))
		{
			movementVector -= get_owner()->get_forward_axis();
		}

		if (Input::GetKey(KeyCode::D))
		{
			movementVector += get_owner()->get_right_axis();
		}

		if (Input::GetKey(KeyCode::A))
		{
			movementVector -= get_owner()->get_right_axis();
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
			movementVector *= mRunMult;
		}

		if (Input::GetKey(KeyCode::LeftAlt))
		{
			movementVector *= mWalkMult;
		}

		get_owner()->translate(movementVector * mSpeed * leopph::time::DeltaTime(), leopph::Space::World);

		auto const [posX, posY] = Input::GetMousePosition();
		auto const diffX = posX - mMouseX;
		auto const diffY = posY - mMouseY;

		short coefficient = 1;
		if (dot(get_owner()->get_up_axis(), leopph::Vector3::up()) < 0)
		{
			coefficient = -1;
		}

		get_owner()->rotate(coefficient * leopph::Vector3::up(), diffX * mMouseSens, leopph::Space::World);
		get_owner()->rotate(leopph::Vector3::right(), diffY * mMouseSens, leopph::Space::Local);

		mMouseX = posX;
		mMouseY = posY;
	}
}
