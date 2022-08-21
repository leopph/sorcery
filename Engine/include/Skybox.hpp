#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <filesystem>


namespace leopph
{
	class Skybox
	{
		public:
			// The 6 faces must be passed in the defined order.
			LEOPPHAPI Skybox(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back);

			Skybox(Skybox const&) = delete;
			void operator=(Skybox const& other) = delete;

			Skybox(Skybox&& other) = delete;
			void operator=(Skybox&& other) = delete;

			LEOPPHAPI ~Skybox();

			LEOPPHAPI u64 get_internal_handle() const;

		private:
			u32 mCubemapName;
			u64 mCubemapHandle;
	};
}
