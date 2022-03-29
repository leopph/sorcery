#define LEOPPH_ENTRY

#include "Init.hpp"

#include "../Constants.hpp"
#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../SceneSwitcher.hpp"
#include "../WindowController.hpp"

#include <Leopph.hpp>


auto leopph::Init() -> void
{
	Window::Instance()->Title(demo::WINDOW_TITLE);

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity{demo::UTILITY_ENTITY_NAME};
	utilEnt->CreateAndAttachComponent<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->CreateAndAttachComponent<Exiter>();
	utilEnt->CreateAndAttachComponent<WindowController>();

	auto const sceneSwitcherEnt = new Entity{demo::SCENE_SWITCHER_ENTITY_NAME};
	auto const sceneSwitcher = sceneSwitcherEnt->CreateAndAttachComponent<demo::SceneSwitcher>();

	auto& churchScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F1);
	auto& spriteScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F2);

	demo::InitChurchScene(churchScene);
	demo::InitSpriteScene(spriteScene);

	churchScene.Deactivate();
	spriteScene.Activate();
	
}
