#define LEOPPH_ENTRY
#include "leopph.h"
#include <iostream>

using namespace leopph;



class FPSCounter : public Behavior
{
private:
	const float m_PollInterval = 0.5f;
	float m_DeltaTime = 0.0f;

public:
	void OnFrameUpdate() override
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
	void OnFrameUpdate() override
	{
		if (Camera::Active() == nullptr)
			return;

		Transform& camTransform = Camera::Active()->Object().Transform();


		if (Input::GetKey(leopph::KeyCode::W))
			camTransform.Translate(camTransform.Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::S))
			camTransform.Translate(-camTransform.Forward() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::D))
			camTransform.Translate(camTransform.Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::A))
			camTransform.Translate(-camTransform.Right() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::E))
			camTransform.Translate(Vector3::Up() * m_Speed * Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::Q))
			camTransform.Translate(Vector3::Down() * m_Speed * Time::DeltaTime());


		std::pair<float, float> mousePos = Input::GetMousePosition();
		float diffX = mousePos.first - lastX;
		float diffY = mousePos.second - lastY;

		camTransform.RotateGlobal(Quaternion{ Vector3::Up(), diffX * m_Sens });
		camTransform.RotateLocal(Quaternion{ Vector3::Right(), diffY * m_Sens });

		lastX = mousePos.first;
		lastY = mousePos.second;
	}
};



class Rotate : public Behavior
{
public:
	void OnFrameUpdate() override
	{
		Object().Transform().RotateGlobal(Quaternion { {0, 1, 0}, 10 * Time::DeltaTime() });
	}
};



void leopph::AppStart()
{
	Input::CursorMode(CursorState::Disabled);

	Object* camera = Object::Create();
	camera->AddComponent<Camera>();
	camera->AddComponent<CameraController>();

	Object* backpack = Object::Create();
	backpack->AddComponent<Model>("models/backpack/backpack.obj");
	backpack->Transform().Position({ 0, 0, 5 });
	backpack->AddComponent<Rotate>();

	Object* pointLightObj = Object::Create();
	pointLightObj->Transform().Position({ 0, 0, 3 });
	pointLightObj->AddComponent<PointLight>();

	Object* fpsCounter = Object::Create();
	fpsCounter->AddComponent<FPSCounter>();
}