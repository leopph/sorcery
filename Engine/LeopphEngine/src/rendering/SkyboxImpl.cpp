#include "rendering/SkyboxImpl.hpp"

#include "Image.hpp"
#include "Logger.hpp"

#include <cstddef>
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
		auto allFilePathStrings{m_AllPaths.string()};
		for (std::size_t separatorPos, faceIndex{0}; (separatorPos = allFilePathStrings.find(PATH_SEPARATOR)) != std::string::npos; allFilePathStrings.erase(0, separatorPos + PATH_SEPARATOR.length()), ++faceIndex)
		{
			m_Paths.at(faceIndex) = allFilePathStrings.substr(0, separatorPos);
		}

		m_Paths.at(5) = std::filesystem::path{allFilePathStrings};

		std::array<Image, 6> faces;

		for (std::size_t i = 0; i < 6; i++)
		{
			faces[i] = Image{m_Paths[i]};
			if (faces[i].Empty())
			{
				auto const errMsg{"Skybox face [" + m_Paths[i].string() + "] could not be loaded."};
				Logger::Instance().Error(errMsg);
			}
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Cubemap);
		glTextureStorage2D(m_Cubemap, 1, GL_RGB8, faces.front().Width(), faces.front().Height());
		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(m_Cubemap, 0, 0, 0, static_cast<GLint>(i), faces[i].Width(), faces[i].Height(), 1, GL_RGB, GL_UNSIGNED_BYTE, faces[i].Data().data());
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


	auto SkyboxImpl::LeftPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[LEFT_PATH_IND];
	}


	auto SkyboxImpl::RightPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[RIGHT_PATH_IND];
	}


	auto SkyboxImpl::TopPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[TOP_PATH_IND];
	}


	auto SkyboxImpl::BottomPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[BOT_PATH_IND];
	}


	auto SkyboxImpl::FrontPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[FRONT_PATH_IND];
	}


	auto SkyboxImpl::BackPath() const noexcept -> std::filesystem::path const&
	{
		return m_Paths[BACK_PATH_IND];
	}


	auto SkyboxImpl::AllPaths() const noexcept -> std::filesystem::path const&
	{
		return m_AllPaths;
	}


	auto SkyboxImpl::BuildAllPaths(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back) -> std::filesystem::path
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


	auto SkyboxImpl::RegisterHandle(Skybox const* const handle) -> void
	{
		m_Handles.push_back(handle);
	}


	auto SkyboxImpl::UnregisterHandle(Skybox const* const handle) -> void
	{
		std::erase(m_Handles, handle);
	}


	auto SkyboxImpl::NumHandles() const -> u64
	{
		return static_cast<u64>(m_Handles.size());
	}


	std::string SkyboxImpl::PATH_SEPARATOR{';'};

	u32 constexpr SkyboxImpl::LEFT_PATH_IND{1};
	u32 constexpr SkyboxImpl::RIGHT_PATH_IND{0};
	u32 constexpr SkyboxImpl::TOP_PATH_IND{2};
	u32 constexpr SkyboxImpl::BOT_PATH_IND{3};
	u32 constexpr SkyboxImpl::FRONT_PATH_IND{4};
	u32 constexpr SkyboxImpl::BACK_PATH_IND{5};

	std::array<f32, 24> const SkyboxImpl::CUBE_VERTICES
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

	std::array<u32, 36> const SkyboxImpl::CUBE_INDICES
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
}
