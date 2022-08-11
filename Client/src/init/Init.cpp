#define LEOPPH_ENTRY

#include "Init.hpp"

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../SceneSwitcher.hpp"
#include "../WindowController.hpp"

#include <Leopph.hpp>


void leopph::Init()
{
	get_window()->set_title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->attach_component<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->attach_component<Exiter>();
	utilEnt->attach_component<WindowController>();

	auto const sceneSwitcher = utilEnt->attach_component<demo::SceneSwitcher>();

	auto& churchScene = sceneSwitcher->CreateOrGetScene(KeyCode::F1);
	auto& spriteScene = sceneSwitcher->CreateOrGetScene(KeyCode::F2);

	InitChurchScene(churchScene);
	//demo::InitSpriteScene(spriteScene);

	churchScene.Activate();
	spriteScene.Deactivate();
}
