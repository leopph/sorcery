#pragma once

#include "Material.hpp"
#include "Vertex.hpp"

#include <memory>
#include <vector>


namespace leopph::internal
{
	// Represents a loaded set of vertices, indices, and materials.
	// It can be used to create renderable objects.
	struct MeshData
	{
		// Vertex data required for rendering.
		// If empty, no primitives will be drawn.
		std::vector<Vertex> Vertices;

		// Vertex indices required for rendering
		// If empty, no primitives will be drawn.
		// An index that points out of the list of vertices is an error and will result in undefined behavior.
		std::vector<unsigned> Indices;

		// Used to define the visual appearance of geometric primitives in the mesh.
		// If no unique customization is required, Materials can be shared between different MeshData objects to achieve a form of instancing.
		// Required for rendering. A nullptr is an error and will result in undefined behavior.
		std::shared_ptr<Material> Material;
	};
}
