#pragma once

#include "../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace internal
	{
		class SkyboxImpl;
	}


	// Represents a cube map that is used to paint the background of a Camera component.
	class Skybox
	{
		public:
			// The 6 faces must be passed in the defined order.
			LEOPPHAPI explicit Skybox(const std::filesystem::path& left, const std::filesystem::path& right,
			                          const std::filesystem::path& top, const std::filesystem::path& bottom,
			                          const std::filesystem::path& front, const std::filesystem::path& back);

			LEOPPHAPI Skybox(const Skybox& other);
			LEOPPHAPI auto operator=(const Skybox& other) -> Skybox&;

			LEOPPHAPI Skybox(Skybox&& other) noexcept;
			LEOPPHAPI auto operator=(Skybox&& other) noexcept -> Skybox&;

			LEOPPHAPI ~Skybox();

			LEOPPHAPI auto RightPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto LeftPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto TopPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto BottomPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto FrontPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto BackPath() const -> const std::filesystem::path&;
			LEOPPHAPI auto AllPaths() const -> const std::filesystem::path&;

		private:
			auto Deinit() -> void;

			internal::SkyboxImpl* m_Impl;
	};
}
