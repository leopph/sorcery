#include "SkyboxResource.hpp"

#include "../util/logger.h"

#include <glad/gl.h>

#include <cstddef>
#include <stb_image.h>
#include <stdexcept>



namespace leopph::impl
{
	SkyboxResource::SkyboxResource(const std::filesystem::path& allFilePaths) :
		UniqueResource{allFilePaths},
		AllFilePaths{allFilePaths},
		RightPath{m_Paths.at(RIGHT)},
		LeftPath{m_Paths.at(LEFT)},
		TopPath{m_Paths.at(TOP)},
		BottomPath{m_Paths.at(BOTTOM)},
		FrontPath{m_Paths.at(FRONT)},
		BackPath{m_Paths.at(BACK)},
		m_TexId{},
		m_Vao{},
		m_Vbo{}
	{
		auto allFilePathStrings{allFilePaths.string()};
		for (std::size_t separatorPos, faceIndex{RIGHT}; (separatorPos = allFilePathStrings.find(FileSeparator)) != std::string::npos; allFilePathStrings.erase(0, separatorPos + FileSeparator.length()), ++faceIndex)
		{
			m_Paths.at(faceIndex) = allFilePathStrings.substr(0, separatorPos);
		}

		m_Paths.at(5) = std::filesystem::path{allFilePathStrings};


		std::array<ImageData, 6> faces;

		for (std::size_t i = 0; i < 6; i++)
		{
			faces.at(i).data = stbi_load(m_Paths.at(i).string().data(), &faces.at(i).width, &faces.at(i).height, &faces.at(i).channels, 0);

			if (faces.at(i).data == nullptr)
			{
				const auto errMsg{"Skybox texture on path [" + m_Paths.at(i).string() + "] could not be loaded."};
				Logger::Instance().Error(errMsg);
				return;
			}
		}


		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TexId);
		glTextureStorage2D(m_TexId, 1, GL_RGB8, faces.front().width, faces.front().height);

		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(m_TexId, 0, 0, 0, static_cast<GLint>(i), faces.at(i).width, faces.at(i).height, 1, GL_RGB, GL_UNSIGNED_BYTE, faces.at(i).data);
		}

		glTextureParameteri(m_TexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_TexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_TexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_TexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_TexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glCreateBuffers(1, &m_Vbo);
		glNamedBufferStorage(m_Vbo, s_CubeVertices.size() * sizeof(decltype(s_CubeVertices)::value_type), s_CubeVertices.data(), 0);

		glCreateVertexArrays(1, &m_Vao);
		glVertexArrayVertexBuffer(m_Vao, 0, m_Vbo, 0, 3 * sizeof(decltype(s_CubeVertices)::value_type));

		glEnableVertexArrayAttrib(m_Vao, 0);
		glVertexArrayAttribFormat(m_Vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_Vao, 0, 0);
	}


	SkyboxResource::~SkyboxResource()
	{
		glDeleteTextures(1, &m_TexId);
		glDeleteBuffers(1, &m_Vbo);
		glDeleteVertexArrays(1, &m_Vao);
	}


	void SkyboxResource::Draw(ShaderProgram& shader) const
	{
		shader.SetUniform("skybox", 0);

		glDisable(GL_CULL_FACE);

		glBindTextureUnit(0, m_TexId);
		glBindVertexArray(m_Vao);
		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(s_CubeVertices.size()));
		glDepthFunc(GL_LESS);
		glBindVertexArray(0);
		glBindTextureUnit(0, 0);

		glEnable(GL_CULL_FACE);
	}


	SkyboxResource::ImageData::~ImageData()
	{
		stbi_image_free(data);
	}


	const std::array<float, 108> SkyboxResource::s_CubeVertices
	{
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
}
