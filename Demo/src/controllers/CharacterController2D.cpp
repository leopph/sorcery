#include "CharacterController2D.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Vector3;
using leopph::Space;


namespace demo
{
	CharacterController2D::CharacterController2D(leopph::CameraNode* camera, float const speed, float const runMult, float const walkMult) :
		mCamera{camera},
		mSpeed{speed},
		mRunMult{runMult},
		mWalkMult{walkMult}
	{ }


	void CharacterController2D::on_frame_update()
	{
		Vector3 posDelta;

		if (Input::get_key(KeyCode::Up) || Input::get_key(KeyCode::W))
		{
			posDelta += Vector3::up();
		}

		if (Input::get_key(KeyCode::Down) || Input::get_key(KeyCode::S))
		{
			posDelta += Vector3::down();
		}

		if (Input::get_key(KeyCode::Left) || Input::get_key(KeyCode::A))
		{
			posDelta += Vector3::left();
		}

		if (Input::get_key(KeyCode::Right) || Input::get_key(KeyCode::D))
		{
			posDelta += Vector3::right();
		}

		posDelta.normalize();

		if (Input::get_key(KeyCode::LeftShift))
		{
			posDelta *= mRunMult;
		}

		if (Input::get_key(KeyCode::LeftAlt))
		{
			posDelta *= mWalkMult;
		}

		mCamera->translate(posDelta * mSpeed * leopph::get_frame_timer().get_last_time(), Space::World);
	}
}
