#pragma once

#include "MeshData.hpp"

#include <memory>
#include <string>


namespace leopph::impl
{
	/* MeshDataGroups are unique stores that contain geometry data.
	 * Their purpose is to cache that data so they are not copyable.
	 * Use shared_ptrs to distribute them. */
	class MeshDataGroup : public std::enable_shared_from_this<MeshDataGroup>
	{
		public:
			explicit MeshDataGroup(std::string id = GenerateId());

			MeshDataGroup(const MeshDataGroup& other) = delete;
			MeshDataGroup& operator=(const MeshDataGroup& other) = delete;

			MeshDataGroup(MeshDataGroup&& other) noexcept = delete;
			MeshDataGroup& operator=(MeshDataGroup&& other) noexcept = delete;

			[[nodiscard]]
			const std::string& Id() const;

			[[nodiscard]]
			const std::vector<MeshData>& Data() const;

			virtual ~MeshDataGroup() noexcept;

		protected:
			[[nodiscard]]
			std::vector<MeshData>& Data();

		private:
			std::string m_Id;
			std::vector<MeshData> m_MeshData;
			
			static std::string GenerateId() noexcept;
	};
}
