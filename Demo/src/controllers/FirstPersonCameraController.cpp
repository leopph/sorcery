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
		mMouseX{Input::get_mouse_position()[0]},
		mMouseY{Input::get_mouse_position()[1]}
	{ }



	void FirstPersonCameraController::on_frame_update()
	{
		leopph::Vector3 movementVector;

		if (Input::get_key(KeyCode::W))
		{
			movementVector += get_owner()->get_forward_axis();
		}

		if (Input::get_key(KeyCode::S))
		{
			movementVector -= get_owner()->get_forward_axis();
		}

		if (Input::get_key(KeyCode::D))
		{
			movementVector += get_owner()->get_right_axis();
		}

		if (Input::get_key(KeyCode::A))
		{
			movementVector -= get_owner()->get_right_axis();
		}

		if (Input::get_key(KeyCode::Space))
		{
			movementVector += leopph::Vector3::up();
		}

		if (Input::get_key(KeyCode::LeftControl))
		{
			movementVector += leopph::Vector3::down();
		}

		movementVector.normalize();

		if (Input::get_key(KeyCode::LeftShift))
		{
			movementVector *= mRunMult;
		}

		if (Input::get_key(KeyCode::LeftAlt))
		{
			movementVector *= mWalkMult;
		}

		get_owner()->translate(movementVector * mSpeed * leopph::get_frame_timer().get_last_time(), leopph::Space::World);

		auto const mousePos = Input::get_mouse_position();
		auto const diffX = mousePos[0] - mMouseX;
		auto const diffY = mousePos[1] - mMouseY;

		short coefficient = 1;
		if (dot(get_owner()->get_up_axis(), leopph::Vector3::up()) < 0)
		{
			coefficient = -1;
		}

		get_owner()->rotate(coefficient * leopph::Vector3::up(), diffX * mMouseSens, leopph::Space::World);
		get_owner()->rotate(leopph::Vector3::right(), diffY * mMouseSens, leopph::Space::Local);

		mMouseX = mousePos[0];
		mMouseY = mousePos[1];
	}
}
