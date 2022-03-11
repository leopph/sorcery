#define LEOPPH_ENTRY

#include "Init.hpp"

#include "Constants.hpp"
#include "Registry.hpp"
#include "SceneSwitcher.hpp"
#include "behaviors/CameraController.hpp"
#include "behaviors/Exiter.hpp"
#include "behaviors/FrameRateAnalyzer.hpp"
#include "behaviors/WindowTester.hpp"

#include <Leopph.hpp>


auto leopph::Init() -> void
{
	Window::Instance()->Title(demo::WINDOW_TITLE);

	Input::CursorMode(CursorState::Disabled);

	const auto player = new Entity{demo::PLAYER_ENTITY_NAME};
	player->CreateAndAttachComponent<CameraController>();
	player->CreateAndAttachComponent<Camera>();

	const auto utilEnt = new Entity{demo::UTILITY_ENTITY_NAME};
	utilEnt->CreateAndAttachComponent<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->CreateAndAttachComponent<Exiter>();
	utilEnt->CreateAndAttachComponent<WindowTester>();

	demo::SceneSwitcher sceneSwitcher;
	const auto churchScene = sceneSwitcher.CreateScene();
	const auto cometScene = sceneSwitcher.CreateScene();

	demo::InitChurchScene(churchScene);
	demo::InitCometScene(cometScene);

	sceneSwitcher.ActivateScene(churchScene);
}
