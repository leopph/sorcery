#pragma once

#include "MaterialData.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <memory>
#include <vector>


namespace leopph
{
	struct StaticMeshData
	{
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		std::shared_ptr<MaterialData> material;
	};
}
