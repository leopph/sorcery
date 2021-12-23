#pragma once

#include "shaders/ShaderProgram.hpp"

#include <array>
#include <filesystem>
#include <string>


namespace leopph::internal
{
	class SkyboxImpl
	{
		public:
			SkyboxImpl(std::filesystem::path allFilePaths);

			SkyboxImpl(const SkyboxImpl& other) = delete;
			SkyboxImpl(SkyboxImpl&& other) = delete;

			~SkyboxImpl();

			auto operator=(const SkyboxImpl& other) -> SkyboxImpl& = delete;
			auto operator=(SkyboxImpl&& other) -> SkyboxImpl = delete;

			auto Draw(ShaderProgram& shader) const -> void;

			// Same as AllFilePaths.
			const std::filesystem::path Path;
			// Same as Path.
			const std::filesystem::path& AllFilePaths;
			const std::filesystem::path& RightPath;
			const std::filesystem::path& LeftPath;
			const std::filesystem::path& TopPath;
			const std::filesystem::path& BottomPath;
			const std::filesystem::path& FrontPath;
			const std::filesystem::path& BackPath;

			inline static const std::string FileSeparator{";"};

		private:
			std::array<std::filesystem::path, 6> m_Paths;
			std::array<unsigned, 4> m_GlNames;

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


			enum
			{
				RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK
			};


			enum
			{
				CUBEMAP, VAO, VBO, EBO
			};


			struct ImageData
			{
				unsigned char* data{nullptr};
				int width{0};
				int height{0};
				int channels{0};

				~ImageData();
			};
	};
}
