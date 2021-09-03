#pragma once

#include "../api/leopphapi.h"
#include "../data/Proxy.hpp"
#include "../rendering/SkyboxHandle.hpp"

#include <concepts>
#include <filesystem>


namespace leopph
{
	namespace impl
	{
		//class SkyboxHandle;
	}

	/*---------------------------------------------------------------------------------------------
	The Skybox class represents a cube map that is used to paint the background of a Camera object.
	It consists of 6 faces, or separate textures files, that MUST BE given in the specified order.
	See "camerabackground.h" for more details.
	---------------------------------------------------------------------------------------------*/
	class Skybox : impl::Proxy<impl::SkyboxHandle>
	{
	public:
		LEOPPHAPI Skybox(const std::filesystem::path& left, const std::filesystem::path& right,
						 const std::filesystem::path& top, const std::filesystem::path& bottom,
						 const std::filesystem::path& back, const std::filesystem::path& front);

		LEOPPHAPI Skybox(const Skybox& other);
		LEOPPHAPI Skybox(Skybox&& other) noexcept;

		LEOPPHAPI Skybox& operator=(const Skybox& other);
		LEOPPHAPI Skybox& operator=(Skybox&& other) noexcept;

		LEOPPHAPI ~Skybox() = default;

		LEOPPHAPI inline const std::filesystem::path& AllFilePaths() const;
		LEOPPHAPI inline const std::filesystem::path& RightPath() const;
		LEOPPHAPI inline const std::filesystem::path& LeftPath() const;
		LEOPPHAPI inline const std::filesystem::path& TopPath() const;
		LEOPPHAPI inline const std::filesystem::path& BottomPath() const;
		LEOPPHAPI inline const std::filesystem::path& FrontPath() const;
		LEOPPHAPI inline const std::filesystem::path& BackPath() const;
	};
}