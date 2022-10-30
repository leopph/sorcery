#pragma once

#include <Entity.hpp>

#include <memory>
#include <span>
#include <ostream>

void Serialize(std::span<std::unique_ptr<leopph::Entity> const> entities, std::ostream& os);