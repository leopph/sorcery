#pragma once

#include "MeshData.hpp"

#include <memory>
#include <string>


namespace leopph::internal
{
	/* MeshDataGroups are unique stores that contain geometry data.
	 * Their purpose is to cache that data so they are not copyable.
	 * Use shared_ptrs to distribute them. */
	class MeshDataGroup : public std::enable_shared_from_this<MeshDataGroup>
	{
		public:
			explicit MeshDataGroup(std::string id = GenerateId());

			MeshDataGroup(const MeshDataGroup& other) = delete;
			auto operator=(const MeshDataGroup& other) -> MeshDataGroup& = delete;

			MeshDataGroup(MeshDataGroup&& other) noexcept = delete;
			auto operator=(MeshDataGroup&& other) noexcept -> MeshDataGroup& = delete;

			auto operator==(const MeshDataGroup& other) const -> bool;

			[[nodiscard]]
			auto Id() const -> const std::string&;

			[[nodiscard]]
			auto Data() const -> const std::vector<MeshData>&;

			virtual ~MeshDataGroup() noexcept;

		protected:
			[[nodiscard]]
			auto Data() -> std::vector<MeshData>&;

		private:
			std::string m_Id;
			std::vector<MeshData> m_MeshData;

			static auto GenerateId() noexcept -> std::string;
	};
}
