#pragma once

#include "Mesh.hpp"
#include "LeopphApi.hpp"

#include <type_traits>
#include <vector>


namespace leopph
{
	// A group of Meshes that logically belong together.
	class MeshGroup
	{
		public:
			// Construct an empty MeshGroup.
			MeshGroup();

			// Takes Meshes from the passed vector.
			explicit MeshGroup(std::vector<Mesh> meshes);

			// Wraps the passed pointer.
			// Passing nullptr is ignored and default construction will take place.
			explicit MeshGroup(std::shared_ptr<std::vector<Mesh>> meshes);

			[[nodiscard]] LEOPPHAPI
			auto Meshes() const noexcept -> std::span<Mesh const>;

			[[nodiscard]] LEOPPHAPI
			auto Meshes() noexcept -> std::vector<Mesh>&;

			// Returns how many times this set of Meshes is used.
			[[nodiscard]] LEOPPHAPI
			auto UseCount() const noexcept -> std::size_t;

		private:
			[[nodiscard]] static
			auto GetMeshesCommon(auto* self) noexcept -> auto&;

			std::shared_ptr<std::vector<Mesh>> m_Meshes;
	};


	static_assert(std::is_default_constructible_v<MeshGroup>);
	static_assert(std::is_copy_constructible_v<MeshGroup>);
	static_assert(std::is_copy_assignable_v<MeshGroup>);
	static_assert(std::is_move_constructible_v<MeshGroup>);
	static_assert(std::is_move_assignable_v<MeshGroup>);
	static_assert(std::is_nothrow_destructible_v<MeshGroup>);


	auto MeshGroup::GetMeshesCommon(auto* const self) noexcept -> auto&
	{
		return *self->m_Meshes;
	}
}
