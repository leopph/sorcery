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
	const auto spotLight = new SpotLight{ *cameraObj };
	spotLight->InnerAngle(15);
	spotLight->OuterAngle(20);

	Camera::Active()->Background(CameraBackground{ .skybox{std::make_unique<Skybox>("skybox/left.jpg", "skybox/right.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/back.jpg", "skybox/front.jpg")}});

	const auto portraitObj = new Object{};
	portraitObj->Transform().Position(Vector3{ 0, 0, 5 });
	portraitObj->Transform().RotateGlobal(Quaternion{ Vector3::Up(), 180 });
	new Model{ *portraitObj, "models/portrait/cropped_textured_mesh.obj" };

	/*constexpr std::size_t cubeNumber{ 5000u };
	for (std::size_t i = 0; i < cubeNumber; i++)
	{
		const ObjectProperties properties
		{
			.isStatic = true,
			.position { Vector3{rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50} }
		};

		const auto cube = new Object{ properties };
		cube->AddComponent<Model>("models/cube/cube.dae");
	}*/

	const auto cubeObj = new Object;
	cubeObj->Transform().Position(Vector3{ 0, 0, 5 });
	new Model{ *cubeObj, "models/cube/cube.dae" };
	new Rotate{ *cubeObj, Vector3::Up(), 30 };

	const auto dirLightObj = new Object{};
	dirLightObj->Transform().RotateGlobal(Quaternion{ Vector3::Up(), 315 });
	dirLightObj->Transform().RotateLocal({ Quaternion{Vector3::Right(), 45} });
	new DirectionalLight{ *dirLightObj };

	new FPSCounter{*new Object};
}