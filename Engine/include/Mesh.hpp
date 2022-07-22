#pragma once

#include "Material.hpp"
#include "Sphere.hpp"
#include "Vertex.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	// Represents a loaded set of vertices, indices, and materials.
	// It can be used to create renderable objects.
	class Mesh
	{
		public:
			Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::shared_ptr<Material> material);

			// Get the vertex data required for rendering.
			// No primitives will be drawn if empty.
			[[nodiscard]]
			std::span<Vertex const> Vertices() const;

			// Set the vertex data required for rendering.
			// No primitives will be drawn if empty.
			void Vertices(std::vector<Vertex> vertices);

			// Get the vertex indices used for rendering.
			// No primitives will be drawn if empty.
			[[nodiscard]]
			std::span<unsigned const> Indices() const;

			// Set the vertex indices used for rendering.
			// No primitives will be drawn if empty.
			void Indices(std::vector<unsigned> indices);

			// Get the material that is used to define the visual appearance of the primitives in the Mesh.
			// If no unique customization is required, Materials can be shared between different Mesh objects to achieve a form of instancing.
			// The returned pointer is never null.
			[[nodiscard]]
			std::shared_ptr<Material> const& Material() const;

			// Set the material that is used to define the visual appearance of the primitives in the Mesh.
			// If no unique customization is required, Materials can be shared between different Mesh objects to achieve a form of instancing.
			// Passing nullptr will be ignored.
			void Material(std::shared_ptr<leopph::Material> material);

			// Get the bounding sphere of the Mesh.
			// This is updated if only if the vertices change.
			// Changing the indices does not affect the bounding sphere.
			[[nodiscard]]
			Sphere const& BoundingSphere() const;

		private:
			// Returns the bounding sphere based on the current vertices.
			[[nodiscard]]
			Sphere CalculateBoundingSphere() const noexcept;

			std::vector<Vertex> m_Vertices;
			std::vector<unsigned> m_Indices;
			std::shared_ptr<leopph::Material> m_Material;
			Sphere m_BoundingSphere{};
	};


	static_assert(!std::is_default_constructible_v<Mesh>);
	static_assert(std::is_copy_constructible_v<Mesh>);
	static_assert(std::is_copy_assignable_v<Mesh>);
	static_assert(std::is_move_constructible_v<Mesh>);
	static_assert(std::is_move_assignable_v<Mesh>);
}
