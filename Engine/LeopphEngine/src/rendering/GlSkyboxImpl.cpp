#include "GlSkyboxImpl.hpp"

#include "Image.hpp"
#include "Logger.hpp"

#include <cstddef>
#include <utility>


namespace leopph::internal
{
	GlSkyboxImpl::GlSkyboxImpl(std::filesystem::path allFilePaths) :
		mAllPaths{std::move(allFilePaths)},
		mVao{},
		mVbo{},
		mIbo{},
		mCubemap{}
	{
		auto allFilePathStrings{mAllPaths.string()};
		for (std::size_t separatorPos, faceIndex{0}; (separatorPos = allFilePathStrings.find(PATH_SEPARATOR)) != std::string::npos; allFilePathStrings.erase(0, separatorPos + PATH_SEPARATOR.length()), ++faceIndex)
		{
			mPaths.at(faceIndex) = allFilePathStrings.substr(0, separatorPos);
		}

		mPaths.at(5) = std::filesystem::path{allFilePathStrings};

		std::array<Image, 6> faces;

		for (std::size_t i = 0; i < 6; i++)
		{
			faces[i] = Image{mPaths[i], ColorEncoding::SRGB};
			if (faces[i].Empty())
			{
				auto const errMsg{"Skybox face [" + mPaths[i].string() + "] could not be loaded."};
				Logger::Instance().Error(errMsg);
			}
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mCubemap);
		glTextureStorage2D(mCubemap, 1, GL_SRGB8, faces.front().Width(), faces.front().Height());
		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(mCubemap, 0, 0, 0, static_cast<GLint>(i), faces[i].Width(), faces[i].Height(), 1, GL_RGB, GL_UNSIGNED_BYTE, faces[i].Data().data());
		}

		glTextureParameteri(mCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(mCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Setup vertex buffer
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, CUBE_VERTICES.size() * sizeof(decltype(CUBE_VERTICES)::value_type), CUBE_VERTICES.data(), 0);

		// Setup index buffer
		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, CUBE_INDICES.size() * sizeof(decltype(CUBE_INDICES)::value_type), CUBE_INDICES.data(), 0);

		// Setup vertex array
		glCreateVertexArrays(1, &mVao);
		glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, 3 * sizeof(decltype(CUBE_VERTICES)::value_type));
		glVertexArrayElementBuffer(mVao, mIbo);

		// Position attribute
		glVertexArrayAttribFormat(mVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(mVao, 0, 0);
		glEnableVertexArrayAttrib(mVao, 0);
	}



	GlSkyboxImpl::~GlSkyboxImpl()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mVbo);
		glDeleteBuffers(1, &mIbo);
		glDeleteTextures(1, &mCubemap);
	}



	void GlSkyboxImpl::draw(gsl::not_null<ShaderProgram const*> const shader) const
	{
		glBindTextureUnit(0, mCubemap);
		shader->set_uniform("u_CubeMap", 0);

		glDisable(GL_CULL_FACE);
		glBindVertexArray(mVao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(CUBE_INDICES.size()), GL_UNSIGNED_INT, nullptr);
	}



	std::filesystem::path const& GlSkyboxImpl::left_path() const
	{
		return mPaths[LEFT_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::right_path() const
	{
		return mPaths[RIGHT_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::top_path() const
	{
		return mPaths[TOP_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::bottom_path() const
	{
		return mPaths[BOT_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::front_path() const
	{
		return mPaths[FRONT_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::back_path() const
	{
		return mPaths[BACK_PATH_IND];
	}



	std::filesystem::path const& GlSkyboxImpl::AllPaths() const
	{
		return mAllPaths;
	}



	std::filesystem::path GlSkyboxImpl::build_all_paths(std::filesystem::path const& left, std::filesystem::path const& right, std::filesystem::path const& top, std::filesystem::path const& bottom, std::filesystem::path const& front, std::filesystem::path const& back)
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



	void GlSkyboxImpl::register_handle(Skybox const* const handle)
	{
		mHandles.push_back(handle);
	}



	void GlSkyboxImpl::unregister_handle(Skybox const* const handle)
	{
		std::erase(mHandles, handle);
	}



	u64 GlSkyboxImpl::num_handles() const
	{
		return mHandles.size();
	}



	std::string GlSkyboxImpl::PATH_SEPARATOR{';'};

	u32 constexpr GlSkyboxImpl::LEFT_PATH_IND{1};
	u32 constexpr GlSkyboxImpl::RIGHT_PATH_IND{0};
	u32 constexpr GlSkyboxImpl::TOP_PATH_IND{2};
	u32 constexpr GlSkyboxImpl::BOT_PATH_IND{3};
	u32 constexpr GlSkyboxImpl::FRONT_PATH_IND{4};
	u32 constexpr GlSkyboxImpl::BACK_PATH_IND{5};

	std::array<f32, 24> const GlSkyboxImpl::CUBE_VERTICES
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

	std::array<u32, 36> const GlSkyboxImpl::CUBE_INDICES
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
