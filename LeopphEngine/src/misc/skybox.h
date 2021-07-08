#pragma once

#include "../api/leopphapi.h"

#include <filesystem>

namespace leopph
{
	namespace impl
	{
		class SkyboxImpl;
	}

	/*---------------------------------------------------------------------------------------------
	The Skybox class represents a cube map that is used to paint the background of a Camera object.
	It consists of 6 faces, or separate textures files, that MUST BE given in the specified order.
	See "camerabackground.h" for more details.
	---------------------------------------------------------------------------------------------*/
	class Skybox
	{
	public:
		LEOPPHAPI Skybox(const std::filesystem::path& left, const std::filesystem::path& right,
			const std::filesystem::path& top, const std::filesystem::path& bottom,
			const std::filesystem::path& back, const std::filesystem::path& front);

		LEOPPHAPI Skybox(const Skybox& other);
		LEOPPHAPI Skybox(Skybox&& other) noexcept;

		LEOPPHAPI ~Skybox();

		LEOPPHAPI Skybox& operator=(const Skybox& other);
		LEOPPHAPI Skybox& operator=(Skybox&& other) noexcept;

		/* Two Skyboxes only compare equal if the order of the given files matches */
		LEOPPHAPI bool operator==(const Skybox& other) const;
		LEOPPHAPI bool operator==(const impl::SkyboxImpl& other) const;

		/* Semicolon separated list of given file names */
		LEOPPHAPI const std::string& FileNames() const;

	private:
		const impl::SkyboxImpl* m_Impl;
	};
}