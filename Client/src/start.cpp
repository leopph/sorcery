#define LEOPPH_ENTRY

#include "Leopph.hpp"
#include "behaviors/CameraController.hpp"
#include "behaviors/Exiter.hpp"
#include "behaviors/FrameRateAnalyzer.hpp"
#include "behaviors/WindowTester.hpp"


auto leopph::Init() -> void
{
	Window::Instance()->Title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	const auto playerEntity = Entity::CreateEntity("player");
	playerEntity->Transform()->Rotate(Vector3::Up(), 90);

	const auto camera{playerEntity->CreateComponent<Camera>()};
	camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr","skybox/megasun/bottom.hdr","skybox/megasun/front.hdr","skybox/megasun/back.hdr"});

	camera->NearClipPlane(0.3f);
	camera->FarClipPlane(1000);

	playerEntity->CreateComponent<CameraController>();

	AmbientLight::Instance().Intensity(Vector3{0.05, 0.05, 0.05});

	const auto dirLightEntity = Entity::CreateEntity("dirlight");
	dirLightEntity->Transform()->Rotate(Vector3::Up(), 135, Space::World);
	dirLightEntity->Transform()->Rotate(Vector3::Right(), 45, Space::Local);
	const auto dirLight = dirLightEntity->CreateComponent<DirectionalLight>();
	dirLight->Diffuse(Vector3{0.07, 0.07, 0.07});
	dirLight->CastsShadow(true);

	const auto group = Entity::CreateEntity();
	group->Transform()->Translate(0, -3, 0);

	const auto church = Entity::CreateEntity("church");
	church->Transform()->Parent(group);
	church->Transform()->Rotate(Vector3::Right(), 90);
	const auto churchModel = church->CreateComponent<Model>("models/church/ChristchurchGreyfriarsRuinGarden03.obj");
	churchModel->CastsShadow(true);

	const auto lamp = Entity::CreateEntity("lamp");
	lamp->Transform()->Parent(group);
	lamp->Transform()->Translate(0, 1.75, 0);
	lamp->Transform()->Rescale(0.01f, 0.01f, 0.01f);
	const auto lampModel = lamp->CreateComponent<Model>("models/lamp/scene.gltf");
	lampModel->CastsShadow(true);

	const auto pLightEntity = Entity::CreateEntity("plight");
	pLightEntity->Transform()->Parent(lamp);
	pLightEntity->Transform()->Translate(-0.5, 3.25, -0.5, Space::Local);
	const auto pLight = pLightEntity->CreateComponent<PointLight>();
	pLight->Range(30);
	pLight->Constant(0.5f);
	pLight->Linear(0.2f);
	pLight->Quadratic(0.07f);
	pLight->CastsShadow(true);
	
	Entity::CreateEntity("fpscounter")->CreateComponent<FrameRateAnalyzer>(0.5f, 60);
	Entity::CreateEntity("windowstester")->CreateComponent<WindowTester>();
	Entity::CreateEntity("exiter")->CreateComponent<Exiter>();
}
