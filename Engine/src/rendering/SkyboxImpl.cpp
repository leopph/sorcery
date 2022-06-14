#include "SkyboxImpl.hpp"

#include "../util/Logger.hpp"

#include <cstddef>
#include <stb_image.h>
#include <utility>


namespace leopph::internal
{
	SkyboxImpl::SkyboxImpl(std::filesystem::path allFilePaths) :
		m_AllPaths{std::move(allFilePaths)},
		m_VertexArray{},
		m_VertexBuffer{},
		m_IndexBuffer{},
		m_Cubemap{}
	{
		stbi_set_flip_vertically_on_load(false);

		auto allFilePathStrings{m_AllPaths.string()};
		for (std::size_t separatorPos, faceIndex{0}; (separatorPos = allFilePathStrings.find(PATH_SEPARATOR)) != std::string::npos; allFilePathStrings.erase(0, separatorPos + PATH_SEPARATOR.length()), ++faceIndex)
		{
			m_Paths.at(faceIndex) = allFilePathStrings.substr(0, separatorPos);
		}

		m_Paths.at(5) = std::filesystem::path{allFilePathStrings};

		std::array<ImageData, 6> faces;

		for (std::size_t i = 0; i < 6; i++)
		{
			faces[i].Data = stbi_load(m_Paths[i].string().data(), &faces[i].Width, &faces[i].Height, &faces[i].Channels, 0);
			if (!faces[i].Data)
			{
				const auto errMsg{"Skybox face [" + m_Paths[i].string() + "] could not be loaded."};
				Logger::Instance().Error(errMsg);
			}
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Cubemap);
		glTextureStorage2D(m_Cubemap, 1, GL_RGB8, faces.front().Width, faces.front().Height);
		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(m_Cubemap, 0, 0, 0, static_cast<GLint>(i), faces[i].Width, faces[i].Height, 1, GL_RGB, GL_UNSIGNED_BYTE, faces[i].Data);
		}

		glTextureParameteri(m_Cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Setup vertex buffer
		glCreateBuffers(1, &m_VertexBuffer);
		glNamedBufferStorage(m_VertexBuffer, CUBE_VERTICES.size() * sizeof(decltype(CUBE_VERTICES)::value_type), CUBE_VERTICES.data(), 0);

		// Setup index buffer
		glCreateBuffers(1, &m_IndexBuffer);
		glNamedBufferStorage(m_IndexBuffer, CUBE_INDICES.size() * sizeof(decltype(CUBE_INDICES)::value_type), CUBE_INDICES.data(), 0);

		// Setup vertex array
		glCreateVertexArrays(1, &m_VertexArray);
		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, 3 * sizeof(decltype(CUBE_VERTICES)::value_type));
		glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);

		// Position attribute
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);
	}


	SkyboxImpl::~SkyboxImpl() noexcept
	{
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(1, &m_VertexBuffer);
		glDeleteBuffers(1, &m_IndexBuffer);
		glDeleteTextures(1, &m_Cubemap);
	}


	auto SkyboxImpl::Draw(ShaderProgram& shader) const -> void
	{
		glBindTextureUnit(0, m_Cubemap);
		shader.SetUniform("u_CubeMap", 0);

		glDisable(GL_CULL_FACE);
		glBindVertexArray(m_VertexArray);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(CUBE_INDICES.size()), GL_UNSIGNED_INT, nullptr);
	}


	auto SkyboxImpl::BuildAllPaths(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& front, const std::filesystem::path& back) -> std::filesystem::path
	{
		return right.string()
		            .append(PATH_SEPARATOR)
		            .append(left.string())
		            .append(PATH_SEPARATOR)
		            .append(top.string())
		            .append(PATH_SEPARATOR)
		            .append(bottom.string())
		            .append(PATH_SEPARATOR)
		            .append(front.string())
		            .append(PATH_SEPARATOR)
		            .append(back.string());
	}


	SkyboxImpl::ImageData::~ImageData() noexcept
	{
		stbi_image_free(Data);
	}
}
