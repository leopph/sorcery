#pragma once

#include "LeopphApi.hpp"
#include "StaticMeshGroupComponent.hpp"

#include <filesystem>


namespace leopph
{
	class StaticModelComponent final : public internal::StaticMeshGroupComponent
	{
		public:
			LEOPPHAPI explicit StaticModelComponent(std::filesystem::path path);

			[[nodiscard]] LEOPPHAPI std::filesystem::path const& path() const;
			
			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

		private:
			std::filesystem::path mPath;
	};
}
