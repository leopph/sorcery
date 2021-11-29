#pragma once

#include "shaders/ShaderProgram.hpp"

#include <array>
#include <filesystem>
#include <string>


namespace leopph::impl
{
	class SkyboxImpl
	{
		public:
			SkyboxImpl(std::filesystem::path allFilePaths);

			SkyboxImpl(const SkyboxImpl& other) = delete;
			SkyboxImpl(SkyboxImpl&& other) = delete;

			~SkyboxImpl();

			SkyboxImpl& operator=(const SkyboxImpl& other) = delete;
			SkyboxImpl operator=(SkyboxImpl&& other) = delete;

			void Draw(ShaderProgram& shader) const;

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
			unsigned m_TexId;
			unsigned m_Vao;
			unsigned m_Vbo;

			static const std::array<float, 108> s_CubeVertices;

			enum {RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK};

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
