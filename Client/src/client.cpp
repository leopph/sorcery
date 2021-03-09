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



class CameraController : public Behavior
{
private:
	const float m_Speed = 2.0f;
	const float m_Sens = 0.1f;
	float lastX = Input::GetMousePosition().first;
	float lastY = Input::GetMousePosition().second;

public:
	using Behavior::Behavior;

	void operator()()
	{
		Camera& cam = Camera::Instance();

		if (Input::GetKey(leopph::KeyCode::W))
			cam.Position(cam.Position() + cam.Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::S))
			cam.Position(cam.Position() - cam.Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::D))
			cam.Position(cam.Position() + cam.Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::A))
			cam.Position(cam.Position() - cam.Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::E))
			cam.Position(cam.Position() + Vector3::Up() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::Q))
			cam.Position(cam.Position() + Vector3::Down() * m_Speed * Time::DeltaTime());


		std::pair<float, float> mousePos = Input::GetMousePosition();
		float diffX = mousePos.first - lastX;
		float diffY = mousePos.second - lastY;

		cam.Rotation(Quaternion{ Vector3::Up(), diffX * m_Sens } * cam.Rotation());
		cam.Rotation(cam.Rotation() * Quaternion{ Vector3::Right(), diffY * m_Sens });

		lastX = mousePos.first;
		lastY = mousePos.second;
	}
};



class Rotate : public Behavior
{
public:
	using Behavior::Behavior;

	void operator()()
	{
		OwningObject().Rotation(OwningObject().Rotation()* Quaternion { {0, 1, 0}, 10 * Time::DeltaTime() });
	}
};



void leopph::Init()
{
	Input::CursorMode(CursorState::Disabled);

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