#pragma once

#include "LeopphApi.hpp"

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
			explicit LEOPPHAPI Skybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& front, const std::filesystem::path& back);

			LEOPPHAPI Skybox(const Skybox& other);
			auto LEOPPHAPI operator=(const Skybox& other) -> Skybox&;

			LEOPPHAPI Skybox(Skybox&& other) noexcept;
			auto LEOPPHAPI operator=(Skybox&& other) noexcept -> Skybox&;

			LEOPPHAPI ~Skybox();

			auto LEOPPHAPI RightPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI LeftPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI TopPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI BottomPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI FrontPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI BackPath() const -> const std::filesystem::path&;
			auto LEOPPHAPI AllPaths() const -> const std::filesystem::path&;

		private:
			auto Deinit() const -> void;

			internal::SkyboxImpl* m_Impl;
	};
}
