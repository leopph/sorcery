#include "leopph.h"
#include "leopphentry.h"

#include <iostream>

using namespace leopph;



class CameraMove : public Behavior
{
private:
	const float m_Speed = 2.0f;

public:
	using Behavior::Behavior;

	void operator()()
	{
		Camera& cam = Camera::Instance();
		
		if (Input::GetKey(leopph::KeyCode::W))
			cam.Position(cam.Position() + Vector3{ 0, 0, -1 } * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::S))
			cam.Position(cam.Position() + Vector3{ 0, 0, 1 } * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::A))
			cam.Position(cam.Position() + Vector3{ -1, 0, 0 } * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::D))
			cam.Position(cam.Position() + Vector3{ 1, 0, 0 } * m_Speed * Time::DeltaTime());
	}
};



class FPSCounter : public Behavior
{
private:
	const float m_PollInterval = 0.5f;
	float m_DeltaTime = 0.0f;

public:
	using Behavior::Behavior;

	void operator()()
	{
		m_DeltaTime += Time::DeltaTime();

		if (m_DeltaTime >= m_PollInterval)
		{
			m_DeltaTime = 0.0f;
			std::cout << "FPS: " << 1 / Time::DeltaTime() << std::endl << "Frametime: " << Time::DeltaTime() * 1000 << " ms" << std::endl << std::endl;
		}
	}
};



class Rotate : public Behavior
{
private:
	const float m_Speed = 45.0f / 2.0f;
	float rotation = 45.0f;

public:
	using Behavior::Behavior;

	void operator()()
	{
		rotation += std::fmod(m_Speed * Time::DeltaTime(), 360.0f);
		this->m_Object.Rotation({ { 1, 1, 1 }, rotation });
	}
};



void leopph::Init()
{
	Object* backpack = Object::Create();
	backpack->AddModel(Model{ "models/backpack/backpack.obj" });
	backpack->Position({ 0, 0, -5 });
	backpack->AddBehavior<Rotate>();

	Object* dirLight = Object::Create();
	dirLight->AddBehavior<DirectionalLight>()->Direction({ -3, -1, -1 });

	Object* cameraMover = Object::Create();
	cameraMover->AddBehavior<CameraMove>();

	Object* fpsCounter = Object::Create();
	fpsCounter->AddBehavior<FPSCounter>();
}