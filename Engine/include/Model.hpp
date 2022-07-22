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

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			// File path of the loaded Model.
			[[nodiscard]] LEOPPHAPI std::filesystem::path const& Path() const noexcept;

		private:
			std::filesystem::path m_Path;
	};
}
