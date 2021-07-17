#define LEOPPH_ENTRY
#include "leopph.h"

#include <iostream>
#include <cstdlib>
#include <memory>

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

	Camera::Active()->Background(CameraBackground{ .skybox{std::make_unique<Skybox>("skybox/left.jpg", "skybox/right.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/back.jpg", "skybox/front.jpg")}});

	auto portrait = Object::Create();
	portrait->AddComponent<Model>("models/portrait/cropped_textured_mesh.obj");

	constexpr const std::size_t cubeNumber{ 5000u };
	for (std::size_t i = 0; i < cubeNumber; i++)
	{
		auto cube = Object::Create();
		cube->Transform().Position({ rand() % 100 - 50, rand() % 100 -50, rand() % 100 - 50 });
		cube->AddComponent<Model>("models/cube/cube.dae");
	}

	Object* dirLight = Object::Create();
	dirLight->AddComponent<DirectionalLight>();
	dirLight->Transform().RotateGlobal(Quaternion{ { 0, 1, 0 }, 225 });
	dirLight->Transform().RotateLocal(Quaternion{ { 1, 0, 0 }, 45 });

	Object* fpsCounter = Object::Create();
	fpsCounter->AddComponent<FPSCounter>();
}