#pragma once

#include "GlCore.hpp"
#include "ShaderProgram.hpp"
#include "Skybox.hpp"
#include "Types.hpp"

#include <gsl/pointers>

#include <array>
#include <filesystem>
#include <string>
#include <vector>


namespace leopph::internal
{
	class SkyboxImpl
	{
		public:
			// Param is in the format that build_all_paths generates.
			explicit SkyboxImpl(std::filesystem::path allFilePaths);


			void draw(gsl::not_null<ShaderProgram const*> shader) const;


			[[nodiscard]] std::filesystem::path const& left_path() const;
			[[nodiscard]] std::filesystem::path const& right_path() const;
			[[nodiscard]] std::filesystem::path const& top_path() const;
			[[nodiscard]] std::filesystem::path const& bottom_path() const;
			[[nodiscard]] std::filesystem::path const& front_path() const;
			[[nodiscard]] std::filesystem::path const& back_path() const;

			// All paths encoded in the format of build_all_paths.
			[[nodiscard]] std::filesystem::path const& AllPaths() const;

			// Encode all paths into one path.
			[[nodiscard]] static std::filesystem::path build_all_paths(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back);


			void register_handle(Skybox const* handle);
			void unregister_handle(Skybox const* handle);
			u64 num_handles() const;


			SkyboxImpl(SkyboxImpl const& other) = delete;
			void operator=(SkyboxImpl const& other) = delete;

			SkyboxImpl(SkyboxImpl&& other) = delete;
			void operator=(SkyboxImpl&& other) = delete;

			~SkyboxImpl();


		private:
			std::filesystem::path mAllPaths;
			std::array<std::filesystem::path, 6> mPaths;
			GLuint mVao;
			GLuint mVbo;
			GLuint mIbo;
			GLuint mCubemap;

			std::vector<Skybox const*> mHandles;

			static std::string PATH_SEPARATOR;

			static u32 const LEFT_PATH_IND;
			static u32 const RIGHT_PATH_IND;
			static u32 const TOP_PATH_IND;
			static u32 const BOT_PATH_IND;
			static u32 const FRONT_PATH_IND;
			static u32 const BACK_PATH_IND;

			static std::array<f32, 24> const CUBE_VERTICES;
			static std::array<u32, 36> const CUBE_INDICES;
	};
}
