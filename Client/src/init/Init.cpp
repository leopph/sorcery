#define LEOPPH_ENTRY

#include "Init.hpp"

#include "../Exiter.hpp"
#include "../FrameRateAnalyzer.hpp"
#include "../SceneSwitcher.hpp"
#include "../WindowController.hpp"

#include <Leopph.hpp>


auto leopph::Init() -> void
{
	Window::Instance()->Title("LeopphEngine Demo");

	Input::CursorMode(CursorState::Disabled);

	auto const utilEnt = new Entity;
	utilEnt->CreateAndAttachComponent<FrameRateAnalyzer>(0.5f, 60u);
	utilEnt->CreateAndAttachComponent<Exiter>();
	utilEnt->CreateAndAttachComponent<WindowController>();

	auto const sceneSwitcher = utilEnt->CreateAndAttachComponent<demo::SceneSwitcher>();

	auto& churchScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F1);
	auto& spriteScene = sceneSwitcher->CreateOrGetScene(leopph::KeyCode::F2);

	demo::InitChurchScene(churchScene);
	demo::InitSpriteScene(spriteScene);

	churchScene.Activate();
	spriteScene.Deactivate();
}
