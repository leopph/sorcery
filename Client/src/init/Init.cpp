#define LEOPPH_ENTRY

#include "Init.hpp"

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../SceneSwitcher.hpp"
#include "../WindowController.hpp"

#include <Leopph.hpp>


auto leopph::Init() -> void
{
	GetWindow()->Title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->create_and_attach_component<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->create_and_attach_component<Exiter>();
	utilEnt->create_and_attach_component<WindowController>();

	auto const sceneSwitcher = utilEnt->create_and_attach_component<demo::SceneSwitcher>();

	auto& churchScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F1);
	auto& spriteScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F2);

	demo::InitChurchScene(churchScene);
	//demo::InitSpriteScene(spriteScene);

	churchScene.Activate();
	spriteScene.Deactivate();
}
