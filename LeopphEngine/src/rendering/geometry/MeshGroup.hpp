#pragma once

#include "Mesh.hpp"

#include <string>
#include <vector>


namespace leopph::internal
{
	// A group of Meshes that logically belong together, identified by an Id.
	struct MeshGroup
	{
		std::string Id; // Shall be unique.
		std::vector<Mesh> Meshes; 	// Shall not be nullptr.
	};
}
