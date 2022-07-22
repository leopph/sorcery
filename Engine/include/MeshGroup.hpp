#pragma once

#include "LeopphApi.hpp"
#include "Mesh.hpp"
#include "Types.hpp"

#include <span>
#include <type_traits>
#include <vector>


namespace leopph
{
	// A group of Meshes that logically belong together.
	class MeshGroup
	{
		public:
			// Construct an empty MeshGroup.
			MeshGroup() = default;

			// Copy meshes from the passed container.
			explicit LEOPPHAPI MeshGroup(std::span<Mesh const> meshes);

			// Move meshes from the passed vector.
			explicit LEOPPHAPI MeshGroup(std::vector<Mesh> meshes);

			// Return the currently stored meshes.
			[[nodiscard]] LEOPPHAPI std::span<Mesh const> Meshes() const noexcept;

			// Append a new Mesh instance to the MeshGroup.
			LEOPPHAPI void AddMesh(Mesh mesh);

			// Remove the Mesh from the specified index.
			// The operation is silently ignored for invalid indices.
			// All meshes following the deleted one will have their indices reduced by one.
			LEOPPHAPI void RemoveMesh(u64 index);

		private:
			std::vector<Mesh> m_Meshes;
	};


	static_assert(std::is_default_constructible_v<MeshGroup>);
	static_assert(std::is_copy_constructible_v<MeshGroup>);
	static_assert(std::is_copy_assignable_v<MeshGroup>);
	static_assert(std::is_move_constructible_v<MeshGroup>);
	static_assert(std::is_move_assignable_v<MeshGroup>);
	static_assert(std::is_nothrow_destructible_v<MeshGroup>);
}
