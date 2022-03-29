#pragma once

#include "../SceneSwitcher.hpp"


namespace demo
{
	// Fills the passed vector with created entities
	auto InitChurchScene(SceneSwitcher::Scene& scene) -> void;
	// Fills the passed vector with created entities
	auto InitSpriteScene(SceneSwitcher::Scene& scene) -> void;
}
