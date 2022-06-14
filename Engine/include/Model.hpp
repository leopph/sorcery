#pragma once

#include "LeopphApi.hpp"
#include "RenderComponent.hpp"

#include <filesystem>


namespace leopph
{
	// A CG model stored on disk in one of the various supported formats.
	class Model final : public internal::RenderComponent
	{
		public:
			LEOPPHAPI explicit Model(std::filesystem::path path);

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			// File path of the loaded Model.
			[[nodiscard]] LEOPPHAPI
			auto Path() const noexcept -> std::filesystem::path const&;

		private:
			std::filesystem::path m_Path;
	};
}
