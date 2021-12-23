#pragma once

#include "Material.hpp"
#include "Vertex.hpp"

#include <memory>
#include <vector>


namespace leopph::internal
{
	/* The m_MeshData class represents a loaded set of vertices, indices, and materials.
	 * It can be used to create renderable objects. */ 
	struct MeshData
	{
		// The vertex data.
		std::vector<Vertex> Vertices;

		// The indices used during rendering. If this is empty, unindexed rendering will be used.
		std::vector<unsigned> Indices;

		/* Materials are used to define the visual appearance of geometric primitives in the mesh.
		 * You can share Materials between different m_MeshData objects to achieve a form of instancing or use separate Materials for unique customizations.
		 * Materials are required for rendering. Settings this pointer to NULL is an error and will result in undefined behavior. */
		std::shared_ptr<Material> Material;
	};
}