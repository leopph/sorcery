#pragma once

#include <Entity.hpp>

#include <span>
#include <ostream>

void Serialize(std::span<leopph::Entity const* const> entities, std::ostream& os);