#pragma once

#include "Skybox.hpp"
#include "Types.hpp"
#include "opengl/OpenGl.hpp"
#include "shaders/ShaderProgram.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <vector>


namespace leopph::internal
{
	class SkyboxImpl
	{
		public:
			// Param is in the format that BuildAllPaths generates.
			explicit SkyboxImpl(std::filesystem::path allFilePaths);

			auto Draw(ShaderProgram& shader) const -> void;

			[[nodiscard]] auto LeftPath() const noexcept -> std::filesystem::path const&;
			[[nodiscard]] auto RightPath() const noexcept -> std::filesystem::path const&;
			[[nodiscard]] auto TopPath() const noexcept -> std::filesystem::path const&;
			[[nodiscard]] auto BottomPath() const noexcept -> std::filesystem::path const&;
			[[nodiscard]] auto FrontPath() const noexcept -> std::filesystem::path const&;
			[[nodiscard]] auto BackPath() const noexcept -> std::filesystem::path const&;

			// All paths encoded in the format of BuildAllPaths.
			[[nodiscard]] auto AllPaths() const noexcept -> std::filesystem::path const&;

			// Encode all paths into one path.
			[[nodiscard]] static auto BuildAllPaths(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back) -> std::filesystem::path;

			auto RegisterHandle(Skybox const* handle) -> void;
			auto UnregisterHandle(Skybox const* handle) -> void;
			auto NumHandles() const -> u64;

			SkyboxImpl(SkyboxImpl const& other) = delete;
			auto operator=(SkyboxImpl const& other) -> void = delete;

			SkyboxImpl(SkyboxImpl&& other) = delete;
			auto operator=(SkyboxImpl&& other) -> void = delete;

			~SkyboxImpl() noexcept;

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
