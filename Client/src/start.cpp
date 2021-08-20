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

	const auto cameraObj = new Object{};
	new Camera{ *cameraObj };
	new CameraController{ *cameraObj };

	Camera::Active()->Background(CameraBackground{ .skybox{std::make_unique<Skybox>("skybox/left.jpg", "skybox/right.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/back.jpg", "skybox/front.jpg")}});

	const auto portraitObj = new Object{};
	new Model{ *portraitObj, "models/portrait/cropped_textured_mesh.obj" };

	constexpr std::size_t cubeNumber{ 1u };
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

	const auto dirLightObj = new Object{};
	new DirectionalLight{ *dirLightObj };
	dirLightObj->Transform().RotateGlobal(Quaternion{ { 0, 1, 0 }, 225 });
	dirLightObj->Transform().RotateLocal(Quaternion{ { 1, 0, 0 }, 45 });

	new FPSCounter{*new Object};
}