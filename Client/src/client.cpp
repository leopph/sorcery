#include <leopph.h>
#include <leopphentry.h>
#include <iostream>

using namespace leopph;



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
		this->OwningObject().Rotation({ Vector3::Up(), rotation });
	}
};



class CameraController : public Behavior
{
private:
	const float m_Speed = 2.0f;
	float lastX{};
	float lastY{};

public:
	using Behavior::Behavior;

	void operator()()
	{
		Camera& cam = Camera::Instance();

		if (Input::GetKey(leopph::KeyCode::W))
			cam.Position(cam.Position() + Vector3::Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::S))
			cam.Position(cam.Position() + -Vector3::Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::A))
			cam.Position(cam.Position() + -Vector3::Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::D))
			cam.Position(cam.Position() + Vector3::Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::Q))
			cam.Position(cam.Position() + Vector3::Up() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::E))
			cam.Position(cam.Position() + -Vector3::Up() * m_Speed * Time::DeltaTime());


		std::pair<float, float> mousePos = Input::GetMousePosition();
		float diffX = mousePos.first - lastX;
		float diffY = mousePos.second - lastY;

		cam.Rotation(cam.Rotation() * Quaternion { Vector3::Up(), diffX });
		cam.Rotation(cam.Rotation()* Quaternion { Vector3::Right(), diffY });

		lastX = mousePos.first;
		lastY = mousePos.second;
	}
};



void leopph::Init()
{
	Object* backpack = Object::Create();
	backpack->AddModel(Model{ "models/backpack/backpack.obj" });
	backpack->Position({ 0, 0, 5 });
	backpack->AddBehavior<Rotate>();

	Object* dirLight = Object::Create();
	dirLight->AddBehavior<DirectionalLight>()->Direction({ -1, 0, 1 });

	Object* fpsCounter = Object::Create();
	fpsCounter->AddBehavior<FPSCounter>();

	Object* cameraController = Object::Create();
	cameraController->AddBehavior<CameraController>();
}