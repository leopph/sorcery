#include "skyboximpl.h"

#include "../misc/skybox.h"

#include "../util/logger.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <cstddef>
#include <stdexcept>

leopph::impl::SkyboxImpl::SkyboxImpl(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front) :
	fileNames{ left.string() + ";" + right.string() + ";" + top.string() + ";" + bottom.string() + ";" + back.string() + ";" + front.string()}
{
	const std::filesystem::path* paths[]{ &right, &left, &top, &bottom, &front, &back };

	struct Face
	{
		unsigned char* data{ nullptr };
		int width{ 0 };
		int height{ 0 };
		int channels{ 0 };

		~Face()
		{
			stbi_image_free(data);
		}
	};

	Face faces[6]{};

	for (unsigned char i = 0; i < 6; ++i)
	{
		faces[i].data = stbi_load(paths[i]->string().data(), &faces[i].width, &faces[i].height, &faces[i].channels, 0);

		if (faces[i].data == nullptr)
		{
			auto msg{ "Skybox texture on path [" + paths[i]->string() + "] could not be loaded." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}
	}

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TexID);

	glTextureStorage2D(m_TexID, 1, GL_RGB8, faces[0].width, faces[0].height);

	for (std::size_t i = 0; i < 6; ++i)
		glTextureSubImage3D(m_TexID, 0, 0, 0, static_cast<GLint>(i), faces[i].width, faces[i].height, 1, GL_RGB, GL_UNSIGNED_BYTE, faces[i].data);

	glTextureParameteri(m_TexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_TexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_TexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_TexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_TexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glCreateBuffers(1, &m_VBO);
	glNamedBufferStorage(m_VBO, s_CubeVertices.size() * sizeof(decltype(s_CubeVertices)::value_type), s_CubeVertices.data(), 0);

	glCreateVertexArrays(1, &m_VAO);
	glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, 3 * sizeof(decltype(s_CubeVertices)::value_type));

	glEnableVertexArrayAttrib(m_VAO, 0);
	glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_VAO, 0, 0);
}

leopph::impl::SkyboxImpl::~SkyboxImpl()
{
	glDeleteTextures(1, &m_TexID);
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

unsigned leopph::impl::SkyboxImpl::ID() const
{
	return m_TexID;
}

void leopph::impl::SkyboxImpl::Draw(const Shader& shader) const
{
	shader.SetUniform("skybox", 0);

	glBindTextureUnit(0, m_TexID);
	glBindVertexArray(m_VAO);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(s_CubeVertices.size()));
	glDepthFunc(GL_LESS);
	glBindVertexArray(0);
	glBindTextureUnit(0, 0);
}

const std::vector<float> leopph::impl::SkyboxImpl::s_CubeVertices
{
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};