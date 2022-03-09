#define LEOPPH_ENTRY

#include "Leopph.hpp"
#include "behaviors/CameraController.hpp"
#include "behaviors/ComponentSwitch.hpp"
#include "behaviors/Exiter.hpp"
#include "behaviors/Flicker.hpp"
#include "behaviors/FrameRateAnalyzer.hpp"
#include "behaviors/WindowTester.hpp"


auto leopph::Init() -> void
{
	Window::Instance()->Title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	const auto playerEntity = new Entity{"player"};
	playerEntity->Transform()->Rotate(Vector3::Up(), 90);

	const auto camera{playerEntity->CreateAndAttachComponent<Camera>()};

	camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"});

	camera->NearClipPlane(0.3f);
	camera->FarClipPlane(1000);

	AmbientLight::Instance().Intensity(Vector3{0.05, 0.05, 0.05});

	const auto dirLightEntity = new Entity{"dirlight"};
	dirLightEntity->Transform()->Rotate(Vector3::Up(), 135, Space::World);
	dirLightEntity->Transform()->Rotate(Vector3::Right(), 45, Space::Local);
	const auto dirLight = dirLightEntity->CreateAndAttachComponent<DirectionalLight>();
	dirLight->Diffuse(Vector3{0.07, 0.07, 0.07});
	dirLight->CastsShadow(true);

	const auto group = new Entity{};
	group->Transform()->Translate(0, -3, 0);

	const auto church = new Entity{"church"};
	church->Transform()->Parent(group);
	church->Transform()->Rotate(Vector3::Right(), 90);
	const auto churchModel = church->CreateAndAttachComponent<Model>("models/church/ChristchurchGreyfriarsRuinGarden03.obj");
	churchModel->CastsShadow(true);

	const auto lamp = new Entity{"lamp"};
	lamp->Transform()->Parent(group);
	lamp->Transform()->Translate(0, 1.75, 0);
	lamp->Transform()->Rescale(0.01f, 0.01f, 0.01f);
	const auto lampModel = lamp->CreateAndAttachComponent<Model>("models/lamp/scene.gltf");
	lampModel->CastsShadow(true);

	const auto pLightEntity = new Entity{"plight"};
	pLightEntity->Transform()->Parent(lamp);
	pLightEntity->Transform()->Translate(-0.5, 3.25, -0.5, Space::Local);
	const auto pLight = pLightEntity->CreateAndAttachComponent<PointLight>();
	pLight->Range(30);
	pLight->Constant(0.5f);
	pLight->Linear(0.2f);
	pLight->Quadratic(0.07f);
	pLight->CastsShadow(true);

	playerEntity->CreateAndAttachComponent<CameraController>();
	(new Entity{"fpscounter"})->CreateAndAttachComponent<FrameRateAnalyzer>(0.5f, 60u);
	(new Entity{"windowstester"})->CreateAndAttachComponent<WindowTester>();
	(new Entity{"exiter"})->CreateAndAttachComponent<Exiter>();
	(new Entity{"componentswitch"})->CreateAndAttachComponent<ComponentSwitch>(std::vector<Component*>{churchModel});
	(new Entity{"flicker"})->CreateAndAttachComponent<demo::Flicker>(pLight, 1.2f, 0.05f);
}
