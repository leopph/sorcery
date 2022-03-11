#pragma once

#include <Leopph.hpp>
#include <vector>


namespace demo
{
	// Fills the passed vector with created entities
	auto InitChurchScene(std::vector<leopph::Entity*>& createdEntities) -> void;
	// Fills the passed vector with created entities
	auto InitCometScene(std::vector<leopph::Entity*>& createdEntities) -> void;
}
