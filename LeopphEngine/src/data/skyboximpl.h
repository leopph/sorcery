#pragma once

#include "../rendering/Shader.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace leopph
{
	class Skybox;

	namespace impl
	{
		class SkyboxImpl
		{
		public:
			SkyboxImpl(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front);

			SkyboxImpl(const SkyboxImpl&) = delete;
			SkyboxImpl(SkyboxImpl&&) = delete;

			~SkyboxImpl();

			void operator=(const SkyboxImpl&) = delete;
			void operator=(SkyboxImpl&&) = delete;

			unsigned ID() const;

			const std::string fileNames;

			void Draw(const Shader& shader) const;

		private:
			unsigned m_TexID;
			unsigned m_VAO;
			unsigned m_VBO;
			static const std::vector<float> s_CubeVertices;
		};
	}
}