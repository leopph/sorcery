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

	const auto groupEntity = new Entity{"group"};
	groupEntity->Transform->Rotate(Vector3::Up(), 180, Space::World);

	const auto playerEntity = new Entity{"player"};
	playerEntity->Transform->Parent(groupEntity);

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
					"skybox/megasun/front.hdr",
					"skybox/megasun/back.hdr"
				}
			}
		});

	playerEntity->AddComponent<CameraController>();

	const auto portraitEntity = new Entity{"portrait"};
	portraitEntity->Transform->Parent(groupEntity);
	portraitEntity->Transform->Rotate(Vector3::Up(), 180);
	portraitEntity->Transform->LocalPosition(Vector3{0, 0, 5});
	const auto portrairModel = portraitEntity->AddComponent<Model>("models/portrait/cropped_textured_mesh.obj");
	portrairModel->CastsShadow(true);

	const auto cubeEntity = new Entity{"cube"};
	cubeEntity->Transform->Parent(groupEntity);
	cubeEntity->Transform->LocalPosition(Vector3{0, 0, 5});
	const auto cubeModel = cubeEntity->AddComponent<Model>("models/cube/cube.dae");
	cubeModel->CastsShadow(true);
	cubeEntity->AddComponent<Rotate>(Vector3::Up(), 30.f);

	const auto dirLightEntity = new Entity{"dirlight"};
	dirLightEntity->Transform->Parent(groupEntity);
	dirLightEntity->Transform->Rotate(Vector3::Up(), 315, Space::World);
	dirLightEntity->Transform->Rotate(Vector3::Right(), 45, Space::Local);
	const auto dirLight = dirLightEntity->AddComponent<DirectionalLight>();
	dirLight->Diffuse(Vector3{0.5, 0.5, 0.5});
	dirLight->CastsShadow(true);

	/*const auto spotLightEntity = new Entity{"spotlight"};
	spotLightEntity->Transform->Parent(groupEntity);
	const auto spotLight = spotLightEntity->AddComponent<SpotLight>();
	spotLight->InnerAngle(45);
	spotLight->OuterAngle(60);
	spotLight->CastsShadow(true);

	const auto pointLightEntity = new Entity{"pointlight"};
	pointLightEntity->Transform->Parent(groupEntity);
	pointLightEntity->Transform->LocalPosition(Vector3{0, 0, 3.5});
	pointLightEntity->Transform->LocalScale(Vector3{0.1, 0.1, 0.1});
	const auto pointLight = pointLightEntity->AddComponent<PointLight>();
	pointLight->Range(15);
	pointLight->CastsShadow(true);*/


	AmbientLight::Instance().Intensity(Vector3{0, 0, 0});

	(new Entity{})->AddComponent<FPSCounter>();
	(new Entity{})->AddComponent<WindowTester>();
}
