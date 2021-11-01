#define LEOPPH_ENTRY

#include "Leopph.hpp"
#include "behaviors/CameraController.hpp"
#include "behaviors/FPSCounter.hpp"
#include "behaviors/Rotate.hpp"
#include "behaviors/WindowTester.hpp"


void leopph::AppStart()
{
	Window::Title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	const auto playerEntity = new Entity{};
	playerEntity->Transform->RotateGlobal(Quaternion{Vector3::Up(), 180});

	const auto camera{playerEntity->AddComponent<Camera>()};
	camera->Background(CameraBackground
		{
			.skybox{
				Skybox
				{
					"skybox/megasun/right.hdr",
					"skybox/megasun/left.hdr",
					"skybox/megasun/top.hdr",
					"skybox/megasun/bottom.hdr",
					"skybox/megasun/back.hdr",
					"skybox/megasun/front.hdr"
				}
			}
		});

	playerEntity->AddComponent<CameraController>();

	const auto portraitEntity = new Entity{};
	portraitEntity->Transform->Position(Vector3{0, 0, -5});
	portraitEntity->AddComponent<Model>("models/portrait/cropped_textured_mesh.obj");

	const auto cubeEntity = new Entity;
	cubeEntity->Transform->Position(Vector3{0, 0, -5});
	const auto cubeModel = cubeEntity->AddComponent<Model>("models/cube/cube.dae");
	cubeModel->CastsShadow(true);
	cubeEntity->AddComponent<Rotate>(Vector3::Up(), 30.f);

	const auto dirLightEntity = new Entity{};
	dirLightEntity->Transform->RotateGlobal(Quaternion{Vector3::Up(), 135});
	dirLightEntity->Transform->RotateLocal({Quaternion{Vector3::Right(), 45}});
	const auto dirLight = dirLightEntity->AddComponent<DirectionalLight>();
	dirLight->Diffuse(Vector3{0.5, 0.5, 0.5});
	dirLight->CastsShadow(true);

	const auto globalSpotLightEntity = new Entity;
	globalSpotLightEntity->Transform->RotateGlobal(Quaternion{Vector3::Up(), 180});
	const auto globalSpotLight = globalSpotLightEntity->AddComponent<SpotLight>();
	globalSpotLight->InnerAngle(45);
	globalSpotLight->OuterAngle(60);
	globalSpotLight->CastsShadow(true);

	const auto pointLightEntity = new Entity{};
	pointLightEntity->Transform->Position(Vector3{0, 0, -3.5});
	const auto pointLight = pointLightEntity->AddComponent<PointLight>();
	pointLight->Range(15);
	pointLight->CastsShadow(true);

	AmbientLight::Instance().Intensity(Vector3{0, 0, 0});

	(new Entity{})->AddComponent<FPSCounter>();
	(new Entity{})->AddComponent<WindowTester>();
}
