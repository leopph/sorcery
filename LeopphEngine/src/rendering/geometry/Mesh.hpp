#pragma once

#include "Material.hpp"
#include "Vertex.hpp"
#include "../../misc/Sphere.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph::internal
{
	// Represents a loaded set of vertices, indices, and materials.
	// It can be used to create renderable objects.
	class Mesh
	{
		public:
			Mesh(std::span<const Vertex> vertices, std::span<const unsigned> indices, std::shared_ptr<Material> material);

			// Get the vertex data required for rendering.
			// No primitives will be drawn if empty.
			[[nodiscard]]
			auto Vertices() const -> std::span<const Vertex>;

			// Set the vertex data required for rendering.
			// No primitives will be drawn if empty.
			auto Vertices(std::span<Vertex> vertices) -> void;

			// Get the vertex indices used for rendering.
			// No primitives will be drawn if empty.
			[[nodiscard]]
			auto Indices() const -> std::span<const unsigned>;

			// Set the vertex indices used for rendering.
			// No primitives will be drawn if empty.
			auto Indices(std::span<unsigned> indices) -> void;

			// Get the material that is used to define the visual appearance of the primitives in the Mesh.
			// If no unique customization is required, Materials can be shared between different Mesh objects to achieve a form of instancing.
			// The returned pointer is never null.
			[[nodiscard]]
			auto Material() const -> const std::shared_ptr<Material>&;

			// Set the material that is used to define the visual appearance of the primitives in the Mesh.
			// If no unique customization is required, Materials can be shared between different Mesh objects to achieve a form of instancing.
			// Passing nullptr will be ignored.
			auto Material(std::shared_ptr<leopph::Material> material) -> void;

			// Get the bounding sphere of the Mesh.
			// This is updated if only if the vertices change.
			// Changing the indices does not affect the bounding sphere.
			[[nodiscard]]
			auto BoundingSphere() const -> const Sphere&;

		private:
			// Returns the bounding sphere based on the current vertices.
			[[nodiscard]]
			auto CalculateBoundingSphere() const noexcept -> Sphere;

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
