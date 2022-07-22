#pragma once

#include "GlCore.hpp"
#include "Skybox.hpp"
#include "Types.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <vector>


namespace leopph::internal
{
	class GlSkyboxImpl
	{
		public:
			// Param is in the format that BuildAllPaths generates.
			explicit GlSkyboxImpl(std::filesystem::path allFilePaths);

			void Draw(ShaderFamily& shader) const;

			[[nodiscard]] std::filesystem::path const& LeftPath() const noexcept;
			[[nodiscard]] std::filesystem::path const& RightPath() const noexcept;
			[[nodiscard]] std::filesystem::path const& TopPath() const noexcept;
			[[nodiscard]] std::filesystem::path const& BottomPath() const noexcept;
			[[nodiscard]] std::filesystem::path const& FrontPath() const noexcept;
			[[nodiscard]] std::filesystem::path const& BackPath() const noexcept;

			// All paths encoded in the format of BuildAllPaths.
			[[nodiscard]] std::filesystem::path const& AllPaths() const noexcept;

			// Encode all paths into one path.
			[[nodiscard]] static std::filesystem::path BuildAllPaths(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back);

			void RegisterHandle(Skybox const* handle);
			void UnregisterHandle(Skybox const* handle);
			u64 NumHandles() const;

			GlSkyboxImpl(GlSkyboxImpl const& other) = delete;
			void operator=(GlSkyboxImpl const& other) = delete;

			GlSkyboxImpl(GlSkyboxImpl&& other) = delete;
			void operator=(GlSkyboxImpl&& other) = delete;

			~GlSkyboxImpl() noexcept;

		private:
			std::filesystem::path m_AllPaths;
			std::array<std::filesystem::path, 6> m_Paths;
			GLuint m_VertexArray;
			GLuint m_VertexBuffer;
			GLuint m_IndexBuffer;
			GLuint m_Cubemap;

			// List of all the Skybox handles referring this impl.
			std::vector<Skybox const*> m_Handles;

			// Used for encoding/decoding paths.
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
