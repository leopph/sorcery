#define LEOPPH_ENTRY
#include "leopph.h"

#include "behaviors/CameraController.hpp"
#include "behaviors/FPSCounter.hpp"
#include "behaviors/Rotate.hpp"

#include <cstdlib>
#include <memory>

void leopph::AppStart()
{
	Input::CursorMode(CursorState::Disabled);

	const auto camera = new Object{};
	camera->AddComponent<Camera>();
	camera->AddComponent<CameraController>();

	Camera::Active()->Background(CameraBackground{ .skybox{std::make_unique<Skybox>("skybox/left.jpg", "skybox/right.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/back.jpg", "skybox/front.jpg")}});

	const auto portrait = new Object{};
	portrait->AddComponent<Model>("models/portrait/cropped_textured_mesh.obj");

	constexpr std::size_t cubeNumber{ 5000u };
	for (std::size_t i = 0; i < cubeNumber; i++)
	{
		const ObjectProperties properties
		{
			.isStatic = true,
			.position { rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50 }
		};

		const auto cube = new Object{ properties };
		cube->AddComponent<Model>("models/cube/cube.dae");
	}

	const auto dirLight = new Object{};
	dirLight->AddComponent<DirectionalLight>();
	dirLight->Transform().RotateGlobal(Quaternion{ { 0, 1, 0 }, 225 });
	dirLight->Transform().RotateLocal(Quaternion{ { 1, 0, 0 }, 45 });

	const auto fpsCounter = new Object{};
	fpsCounter->AddComponent<FPSCounter>();
}