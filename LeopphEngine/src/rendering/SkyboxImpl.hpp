#pragma once

#include "shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <array>
#include <filesystem>
#include <string>


namespace leopph::internal
{
	class SkyboxImpl
	{
		public:
			// Param is in the format that BuildAllPaths generates.
			explicit SkyboxImpl(std::filesystem::path allFilePaths);

			SkyboxImpl(const SkyboxImpl& other) = delete;
			auto operator=(const SkyboxImpl& other) -> SkyboxImpl& = delete;

			SkyboxImpl(SkyboxImpl&& other) = delete;
			auto operator=(SkyboxImpl&& other) -> SkyboxImpl = delete;

			~SkyboxImpl() noexcept;

			auto Draw(ShaderProgram& shader) const -> void;

			[[nodiscard]] constexpr auto LeftPath() const noexcept -> auto&;
			[[nodiscard]] constexpr auto RightPath() const noexcept -> auto&;
			[[nodiscard]] constexpr auto TopPath() const noexcept -> auto&;
			[[nodiscard]] constexpr auto BottomPath() const noexcept -> auto&;
			[[nodiscard]] constexpr auto FrontPath() const noexcept -> auto&;
			[[nodiscard]] constexpr auto BackPath() const noexcept -> auto&;
			// All paths encoded in the format of BuildAllPaths.
			[[nodiscard]] constexpr auto Path() const noexcept -> auto&;

			// Encode all paths into one path.
			[[nodiscard]] static auto BuildAllPaths(const std::filesystem::path& left, const std::filesystem::path& right,
			                                        const std::filesystem::path& top, const std::filesystem::path& bottom,
			                                        const std::filesystem::path& front, const std::filesystem::path& back) -> std::filesystem::path;

		private:
			std::filesystem::path m_AllPaths;
			std::array<std::filesystem::path, 6> m_Paths;
			GLuint m_VertexArray;
			GLuint m_VertexBuffer;
			GLuint m_IndexBuffer;
			GLuint m_Cubemap;

			// Used for encoding/decoding paths.
			inline static std::string PATH_SEPARATOR{";"};

			static constexpr unsigned LEFT_PATH_IND{1};
			static constexpr unsigned RIGHT_PATH_IND{0};
			static constexpr unsigned TOP_PATH_IND{2};
			static constexpr unsigned BOT_PATH_IND{3};
			static constexpr unsigned FRONT_PATH_IND{4};
			static constexpr unsigned BACK_PATH_IND{5};

			static constexpr std::array CUBE_VERTICES
			{
				-1.f, 1.f, 1.f, // 0 left-top-front
				-1.f, -1.f, 1.f, // 1 left-bottom-front
				1.f, 1.f, 1.f, // 2 right-top-front
				1.f, -1.f, 1.f, // 3 right-bottom-front
				1.f, 1.f, -1.f, // 4 right-top-back
				1.f, -1.f, -1.f, // 5 right-bottom-back
				-1.f, 1.f, -1.f, // 6 left-top-back
				-1.f, -1.f, -1.f  // 7 left-bottom-back
			};

			static constexpr std::array CUBE_INDICES
			{
				0u, 1u, 2u, // front upper
				1u, 2u, 3u, // front lower
				2u, 3u, 4u, // right upper
				3u, 4u, 5u, // right lower
				4u, 5u, 6u, // right upper
				5u, 6u, 7u, // right lower
				6u, 7u, 0u, // left upper
				7u, 0u, 1u, // left lower
				0u, 2u, 6u, // top upper
				2u, 6u, 4u, // top lower
				1u, 3u, 5u, // bottom upper
				1u, 5u, 7u  // bottom lower
			};


			struct ImageData
			{
				unsigned char* Data{nullptr};
				int Width{0};
				int Height{0};
				int Channels{0};

				ImageData() = default;
				ImageData(const ImageData& other) = default;
				auto operator=(const ImageData& other) -> ImageData& = default;
				ImageData(ImageData&& other) noexcept = default;
				auto operator=(ImageData&& other) noexcept -> ImageData& = default;
				~ImageData() noexcept;
			};
	};


	constexpr auto SkyboxImpl::LeftPath() const noexcept -> auto&
	{
		return m_Paths[LEFT_PATH_IND];
	}


	constexpr auto SkyboxImpl::RightPath() const noexcept -> auto&
	{
		return m_Paths[RIGHT_PATH_IND];
	}


	constexpr auto SkyboxImpl::TopPath() const noexcept -> auto&
	{
		return m_Paths[TOP_PATH_IND];
	}


	constexpr auto SkyboxImpl::BottomPath() const noexcept -> auto&
	{
		return m_Paths[BOT_PATH_IND];
	}


	constexpr auto SkyboxImpl::FrontPath() const noexcept -> auto&
	{
		return m_Paths[FRONT_PATH_IND];
	}


	constexpr auto SkyboxImpl::BackPath() const noexcept -> auto&
	{
		return m_Paths[BACK_PATH_IND];
	}


	constexpr auto SkyboxImpl::Path() const noexcept -> auto&
	{
		return m_AllPaths;
	}
}
