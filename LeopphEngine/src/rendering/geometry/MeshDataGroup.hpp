#pragma once

#include "MeshData.hpp"

#include <memory>
#include <string>


namespace leopph::impl
{
	class MeshDataGroup
	{
		public:
			explicit MeshDataGroup(std::string id = GenerateId());

			[[nodiscard]]
			const std::string& Id() const;

			[[nodiscard]]
			const std::vector<MeshData>& Data() const;

		protected:
			[[nodiscard]]
			std::vector<MeshData>& Data();

		private:
			std::string m_Id;
			std::shared_ptr<std::vector<MeshData>> m_MeshData{std::make_shared_for_overwrite<std::vector<MeshData>>()};

			static std::string GenerateId() noexcept;
	};
}
