#pragma once

#include "../rendering/shader.h"

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

		struct SkyboxImplHash
		{
			using is_transparent = void;

			std::size_t operator()(const SkyboxImpl& skyboxImpl) const;
			std::size_t operator()(const std::string& string) const;
			std::size_t operator()(const Skybox& skybox) const;
		};

		struct SkyboxImplEqual
		{
			using is_transparent = void;

			bool operator()(const SkyboxImpl& left, const SkyboxImpl& right) const;
			bool operator()(const std::string& left, const SkyboxImpl& right) const;
			bool operator()(const SkyboxImpl& left, const std::string& right) const;
			bool operator()(const SkyboxImpl& left, const Skybox& right) const;
			bool operator()(const Skybox& left, const SkyboxImpl& right) const;
		};
	}
}