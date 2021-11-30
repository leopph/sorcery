#include "SkyboxImpl.hpp"

#include "../util/logger.h"

#include <glad/gl.h>
#include <stb_image.h>

#include <cstddef>
#include <utility>


namespace leopph::impl
{
	SkyboxImpl::SkyboxImpl(std::filesystem::path allFilePaths) :
		Path{std::move(allFilePaths)},
		AllFilePaths{Path},
		RightPath{m_Paths.at(RIGHT)},
		LeftPath{m_Paths.at(LEFT)},
		TopPath{m_Paths.at(TOP)},
		BottomPath{m_Paths.at(BOTTOM)},
		FrontPath{m_Paths.at(FRONT)},
		BackPath{m_Paths.at(BACK)},
		m_GlNames{}
	{
		auto allFilePathStrings{AllFilePaths.string()};
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


		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_GlNames[CUBEMAP]);
		glTextureStorage2D(m_GlNames[CUBEMAP], 1, GL_RGB8, faces.front().width, faces.front().height);

		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(m_GlNames[CUBEMAP], 0, 0, 0, static_cast<GLint>(i), faces.at(i).width, faces.at(i).height, 1, GL_RGB, GL_UNSIGNED_BYTE, faces.at(i).data);
		}

		glTextureParameteri(m_GlNames[CUBEMAP], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_GlNames[CUBEMAP], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glCreateBuffers(2, &m_GlNames[VBO]);
		glNamedBufferStorage(m_GlNames[VBO], CUBE_VERTICES.size() * sizeof(decltype(CUBE_VERTICES)::value_type), CUBE_VERTICES.data(), 0);
		glNamedBufferStorage(m_GlNames[EBO], CUBE_INDICES.size() * sizeof(decltype(CUBE_INDICES)::value_type), CUBE_INDICES.data(), 0);

		glCreateVertexArrays(1, &m_GlNames[VAO]);

		glVertexArrayVertexBuffer(m_GlNames[VAO], 0, m_GlNames[VBO], 0, 3 * sizeof(decltype(CUBE_VERTICES)::value_type));
		glVertexArrayAttribFormat(m_GlNames[VAO], 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_GlNames[VAO], 0, 0);
		glEnableVertexArrayAttrib(m_GlNames[VAO], 0);

		glVertexArrayElementBuffer(m_GlNames[VAO], m_GlNames[EBO]);
	}


	SkyboxImpl::~SkyboxImpl()
	{
		glDeleteTextures(1, &m_GlNames[CUBEMAP]);
		glDeleteBuffers(2, &m_GlNames[VBO]);
		glDeleteVertexArrays(1, &m_GlNames[VAO]);
	}


	void SkyboxImpl::Draw(ShaderProgram& shader) const
	{
		shader.SetUniform("skybox", 0);

		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);

		glBindTextureUnit(0, m_GlNames[CUBEMAP]);
		glBindVertexArray(m_GlNames[VAO]);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(CUBE_INDICES.size()), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glBindTextureUnit(0, 0);

		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
	}


	SkyboxImpl::ImageData::~ImageData()
	{
		stbi_image_free(data);
	}
}
