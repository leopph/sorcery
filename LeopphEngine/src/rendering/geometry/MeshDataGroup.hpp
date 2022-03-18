#pragma once

#include "MeshData.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>


namespace leopph::internal
{
	/* MeshDataGroups are unique stores that contain geometry data.
	 * Their purpose is to cache that data so they are not copyable.
	 * Use shared_ptrs to distribute them. */
	class MeshDataGroup : public std::enable_shared_from_this<MeshDataGroup>
	{
		public:
			explicit MeshDataGroup(std::string id);

			MeshDataGroup(const MeshDataGroup& other) = delete;
			auto operator=(const MeshDataGroup& other) -> MeshDataGroup& = delete;

			MeshDataGroup(MeshDataGroup&& other) noexcept = delete;
			auto operator=(MeshDataGroup&& other) noexcept -> MeshDataGroup& = delete;

			// Provides ordering based on id.
			[[nodiscard]] constexpr
			auto operator<=>(const MeshDataGroup& other) const;

			// Equality based on id.
			[[nodiscard]] constexpr
			auto operator==(const MeshDataGroup& other) const -> bool;

			[[nodiscard]] constexpr
			auto Id() const -> std::string_view;

			[[nodiscard]] constexpr
			auto Data() const -> std::span<const MeshData>;

			virtual ~MeshDataGroup() noexcept;

		protected:
			std::vector<MeshData> m_MeshData;

		private:
			std::string m_Id;
	};


	constexpr auto MeshDataGroup::operator<=>(const MeshDataGroup& other) const
	{
		return m_Id <=> other.m_Id;
	}


	constexpr auto MeshDataGroup::operator==(const MeshDataGroup& other) const -> bool
	{
		return m_Id == other.m_Id;
	}


	constexpr auto MeshDataGroup::Id() const -> std::string_view
	{
		return m_Id;
	}


	constexpr auto MeshDataGroup::Data() const -> std::span<const MeshData>
	{
		return m_MeshData;
	}
}
