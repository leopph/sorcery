#pragma once

#include "../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace internal
	{
		class SkyboxImpl;
	}


	/* The Skybox class represents a cube map that is used to paint the background of a Camera component.
	 * It consists of 6 faces, or separate textures files, that MUST BE given in the specified order.
	 * See "camerabackground.h" for more details. */
	class Skybox
	{
		public:
			LEOPPHAPI explicit Skybox(const std::filesystem::path& right, const std::filesystem::path& left,
			                          const std::filesystem::path& top, const std::filesystem::path& bottom,
			                          const std::filesystem::path& front, const std::filesystem::path& back);
			LEOPPHAPI Skybox(const Skybox& other);
			LEOPPHAPI Skybox(Skybox&& other) noexcept;

			LEOPPHAPI ~Skybox();

			LEOPPHAPI auto operator=(const Skybox& other) -> Skybox&;
			LEOPPHAPI auto operator=(Skybox&& other) noexcept -> Skybox&;

			LEOPPHAPI inline auto AllFilePaths() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto RightPath() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto LeftPath() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto TopPath() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto BottomPath() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto FrontPath() const -> const std::filesystem::path&;
			LEOPPHAPI inline auto BackPath() const -> const std::filesystem::path&;

		private:
			auto Deinit() -> void;

			internal::SkyboxImpl* m_Impl;
	};
}
