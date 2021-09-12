#define LEOPPH_ENTRY
#include "leopph.h"

#include "behaviors/CameraController.hpp"
#include "behaviors/FPSCounter.hpp"
#include "behaviors/Rotate.hpp"

#include <memory>


void leopph::AppStart()
{
	Input::CursorMode(CursorState::Disabled);

	const auto cameraObj = new Entity{};
	cameraObj->Transform().RotateGlobal(Quaternion{Vector3::Up(), 180});
	new Camera{ *cameraObj };
	new CameraController{ *cameraObj };
	const auto spotLight = new SpotLight{ *cameraObj };
	spotLight->InnerAngle(15);
	spotLight->OuterAngle(20);

	Camera::Active()->Background(CameraBackground{ .skybox{std::make_unique<Skybox>("skybox/megasun/right.hdr", "skybox/megasun/left.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/back.hdr", "skybox/megasun/front.hdr")}});

	const auto portraitObj = new Entity{};
	portraitObj->Transform().Position(Vector3{ 0, 0, -5 });
	new Model{ *portraitObj, "models/portrait/cropped_textured_mesh.obj" };

	const auto cubeObj = new Entity;
	cubeObj->Transform().Position(Vector3{ 0, 0, -5 });
	new Model{*cubeObj, "models/cube/cube.dae"};
	new Rotate{*cubeObj, Vector3::Up(), 30};

	const auto dirLightObj = new Entity{};
	dirLightObj->Transform().RotateGlobal(Quaternion{ Vector3::Up(), 135 });
	dirLightObj->Transform().RotateLocal({ Quaternion{Vector3::Right(), 45} });
	const auto dirLight = new DirectionalLight{ *dirLightObj };
	dirLight->Diffuse(Vector3{0.5, 0.5, 0.5});

	new FPSCounter{*new Entity};
}