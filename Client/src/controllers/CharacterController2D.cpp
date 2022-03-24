#include "CharacterController2D.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Vector3;
using leopph::time::DeltaTime;
using leopph::Space;


namespace demo
{
	CharacterController2D::CharacterController2D(leopph::Transform* target, float const speed, float const runMult, float const walkMult) :
		m_Target{target},
		m_Speed{speed},
		m_RunMult{runMult},
		m_WalkMult{walkMult}
	{ }


	auto CharacterController2D::OnFrameUpdate() -> void
	{
		Vector3 posDelta;

		if (Input::GetKey(KeyCode::Up) || Input::GetKey(KeyCode::W))
		{
			posDelta += Vector3::Up();
		}

		if (Input::GetKey(KeyCode::Down) || Input::GetKey(KeyCode::S))
		{
			posDelta += Vector3::Down();
		}

		if (Input::GetKey(KeyCode::Left) || Input::GetKey(KeyCode::A))
		{
			posDelta += Vector3::Left();
		}

		if (Input::GetKey(KeyCode::Right) || Input::GetKey(KeyCode::D))
		{
			posDelta += Vector3::Right();
		}

		if (Input::GetKey(KeyCode::LeftShift))
		{
			posDelta *= m_RunMult;
		}

		if (Input::GetKey(KeyCode::LeftAlt))
		{
			posDelta *= m_WalkMult;
		}

		m_Target->Translate(posDelta * DeltaTime(), Space::World);
	}
}
