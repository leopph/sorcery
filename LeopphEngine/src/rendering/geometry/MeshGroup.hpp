#pragma once

#include "Mesh.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>


namespace leopph::internal
{
	// A collection of Meshes identified by a unique handle.
	class MeshGroup : public std::enable_shared_from_this<MeshGroup>
	{
		public:
			explicit MeshGroup(std::string id);

			MeshGroup(const MeshGroup& other) = delete;
			auto operator=(const MeshGroup& other) -> MeshGroup& = delete;

			MeshGroup(MeshGroup&& other) noexcept = delete;
			auto operator=(MeshGroup&& other) noexcept -> MeshGroup& = delete;

			// Provides ordering based on id.
			[[nodiscard]] constexpr
			auto operator<=>(const MeshGroup& other) const;

			// Equality based on id.
			[[nodiscard]] constexpr
			auto operator==(const MeshGroup& other) const -> bool;

			[[nodiscard]] constexpr
			auto Id() const -> std::string_view;

			[[nodiscard]] constexpr
			auto Data() const -> std::span<const Mesh>;

			virtual ~MeshGroup() noexcept;

		protected:
			std::vector<Mesh> m_MeshData;

		private:
			std::string m_Id;
	};


	constexpr auto MeshGroup::operator<=>(const MeshGroup& other) const
	{
		return m_Id <=> other.m_Id;
	}


	constexpr auto MeshGroup::operator==(const MeshGroup& other) const -> bool
	{
		return m_Id == other.m_Id;
	}


	constexpr auto MeshGroup::Id() const -> std::string_view
	{
		return m_Id;
	}


	constexpr auto MeshGroup::Data() const -> std::span<const Mesh>
	{
		return m_MeshData;
	}
}
