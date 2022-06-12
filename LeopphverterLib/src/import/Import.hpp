#pragma once

#include "../Common.hpp"

#include <filesystem>
#include <vector>


namespace leopph::convert
{
	std::vector<Mesh> Import(std::filesystem::path const& path);
}