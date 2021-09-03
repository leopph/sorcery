#pragma once

#include "../api/leopphapi.h"
#include "../data/managed/ResourceHandle.hpp"
#include "SkyboxResource.hpp"

#include <filesystem>


namespace leopph::impl
{
	class SkyboxHandle : public ResourceHandle<SkyboxResource>
	{
	public:
		explicit SkyboxHandle(const std::filesystem::path& right, const std::filesystem::path& left,
							  const std::filesystem::path& top, const std::filesystem::path& bottom,
							  const std::filesystem::path& front,const std::filesystem::path& back);

		SkyboxHandle(const SkyboxHandle& other);
		SkyboxHandle(SkyboxHandle&& other) noexcept;

		SkyboxHandle& operator=(const SkyboxHandle& other);
		SkyboxHandle& operator=(SkyboxHandle&& other) noexcept;

		~SkyboxHandle() = default;

		LEOPPHAPI inline const std::filesystem::path& AllFilePaths() const;
		LEOPPHAPI inline const std::filesystem::path& RightPath() const;
		LEOPPHAPI inline const std::filesystem::path& LeftPath() const;
		LEOPPHAPI inline const std::filesystem::path& TopPath() const;
		LEOPPHAPI inline const std::filesystem::path& BottomPath() const;
		LEOPPHAPI inline const std::filesystem::path& FrontPath() const;
		LEOPPHAPI inline const std::filesystem::path& BackPath() const;
	};
}