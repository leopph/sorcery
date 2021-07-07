#pragma once

#include "../api/leopphapi.h"

#include <filesystem>

namespace leopph
{
	namespace impl
	{
		class SkyboxImpl;
	}


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

		LEOPPHAPI bool operator==(const Skybox& other) const;
		LEOPPHAPI bool operator==(const impl::SkyboxImpl& other) const;

		LEOPPHAPI const std::string& FileNames() const;

	private:
		const impl::SkyboxImpl* m_Impl;
	};
}