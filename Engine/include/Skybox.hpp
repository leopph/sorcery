#pragma once

#include "LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace internal
	{
		class GlSkyboxImpl;
	}


	// Represents a cube map that is used to paint the background of a Camera component.
	class Skybox
	{
		public:
			// The 6 faces must be passed in the defined order.
			explicit LEOPPHAPI Skybox(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back);

			LEOPPHAPI Skybox(Skybox const& other);
			LEOPPHAPI Skybox& operator=(Skybox const& other);

			LEOPPHAPI Skybox(Skybox&& other) noexcept;
			LEOPPHAPI Skybox& operator=(Skybox&& other) noexcept;

			LEOPPHAPI ~Skybox();

			LEOPPHAPI const std::filesystem::path& RightPath() const;
			LEOPPHAPI const std::filesystem::path& LeftPath() const;
			LEOPPHAPI const std::filesystem::path& TopPath() const;
			LEOPPHAPI const std::filesystem::path& BottomPath() const;
			LEOPPHAPI const std::filesystem::path& FrontPath() const;
			LEOPPHAPI const std::filesystem::path& BackPath() const;
			LEOPPHAPI const std::filesystem::path& AllPaths() const;

		private:
			void Deinit() const;

			internal::GlSkyboxImpl* m_Impl;
	};
}
