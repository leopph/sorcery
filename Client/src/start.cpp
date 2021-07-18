#define LEOPPH_ENTRY
#include "leopph.h"

#include "behaviors/cameracontroller.h"
#include "behaviors/fpscounter.h"
#include "behaviors/rotate.h"

#include <cstdlib>
#include <memory>

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
		cube->AddComponent<Rotate>();
	}

	Object* dirLight = Object::Create();
	dirLight->AddComponent<DirectionalLight>();
	dirLight->Transform().RotateGlobal(Quaternion{ { 0, 1, 0 }, 225 });
	dirLight->Transform().RotateLocal(Quaternion{ { 1, 0, 0 }, 45 });

	Object* fpsCounter = Object::Create();
	fpsCounter->AddComponent<FPSCounter>();
}