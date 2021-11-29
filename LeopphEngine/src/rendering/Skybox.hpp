#pragma once

#include "../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace impl
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
							  const std::filesystem::path& front,const std::filesystem::path& back);
		LEOPPHAPI Skybox(const Skybox& other);
		LEOPPHAPI Skybox(Skybox&& other) noexcept;

		LEOPPHAPI ~Skybox();

		LEOPPHAPI Skybox& operator=(const Skybox& other);
		LEOPPHAPI Skybox& operator=(Skybox&& other) noexcept;

		LEOPPHAPI inline const std::filesystem::path& AllFilePaths() const;
		LEOPPHAPI inline const std::filesystem::path& RightPath() const;
		LEOPPHAPI inline const std::filesystem::path& LeftPath() const;
		LEOPPHAPI inline const std::filesystem::path& TopPath() const;
		LEOPPHAPI inline const std::filesystem::path& BottomPath() const;
		LEOPPHAPI inline const std::filesystem::path& FrontPath() const;
		LEOPPHAPI inline const std::filesystem::path& BackPath() const;


	private:
		void Deinit();

		impl::SkyboxImpl* m_Impl;
	};
}