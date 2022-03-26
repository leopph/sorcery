#define LEOPPH_ENTRY

#include "Init.hpp"

#include "../Constants.hpp"
#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../SceneSwitcher.hpp"
#include "../TeleportGate.hpp"
#include "../WindowController.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>


auto leopph::Init() -> void
{
	Window::Instance()->Title(demo::WINDOW_TITLE);

	Input::CursorMode(CursorState::Disabled);

	auto const camEntity = new Entity{demo::CAM_ENTITY_NAME};
	auto const perspectiveCamera = camEntity->CreateAndAttachComponent<PerspectiveCamera>();
	camEntity->CreateAndAttachComponent<demo::FirstPersonCameraController>(perspectiveCamera, demo::CAM_3D_SPEED, demo::CAM_3D_SENS, demo::CAM_3D_RUN_MULT, demo::CAM_3D_WALK_MULT);

	auto const utilEnt = new Entity{demo::UTILITY_ENTITY_NAME};
	utilEnt->CreateAndAttachComponent<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->CreateAndAttachComponent<Exiter>();
	utilEnt->CreateAndAttachComponent<WindowController>();

	auto const sceneSwitcherEnt = new Entity{demo::SCENE_SWITCHER_ENTITY_NAME};
	auto const sceneSwitcher = sceneSwitcherEnt->CreateAndAttachComponent<demo::SceneSwitcher>();

	auto const teleport = new Entity{demo::TELEPORT_ENTITY_NAME};
	teleport->CreateAndAttachComponent<demo::TeleportGate>(camEntity, sceneSwitcher);

	auto const churchScene = sceneSwitcher->CreateScene();
	auto const cometScene = sceneSwitcher->CreateScene();
	auto const spriteScene = sceneSwitcher->CreateScene();

	//demo::InitChurchScene(churchScene, cometScene);
	//demo::InitCometScene(cometScene);
	demo::InitSpriteScene(spriteScene);

	sceneSwitcher->ActivateScene(spriteScene);
}
