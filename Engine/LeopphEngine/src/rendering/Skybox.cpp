#include "Skybox.hpp"

#include "GlCore.hpp"
#include "Image.hpp"


namespace leopph
{
	Skybox::Skybox(std::filesystem::path const& left, std::filesystem::path const& right,
	               std::filesystem::path const& top, std::filesystem::path const& bottom,
	               std::filesystem::path const& front, std::filesystem::path const& back)
	{
		std::vector<Image> faces;
		faces.reserve(6);

		for (auto const& path : {left, right, top, bottom, front, back})
		{
			faces.emplace_back(path, ColorEncoding::sRGB);
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mCubemapName);
		glTextureStorage2D(mCubemapName, 1, GL_SRGB8, faces.front().get_width(), faces.front().get_height());

		for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(mCubemapName, 0, 0, 0, static_cast<GLint>(i), faces[i].get_width(), faces[i].get_height(), 1, GL_RGB, GL_UNSIGNED_BYTE, faces[i].get_data().data());
		}

		mCubemapHandle = glGetTextureHandleARB(mCubemapName);
		glMakeTextureHandleResidentARB(mCubemapHandle);
	}



	Skybox::~Skybox()
	{
		glMakeTextureHandleNonResidentARB(mCubemapHandle);
		glDeleteTextures(1, &mCubemapName);
	}



	u32 Skybox::get_internal_handle() const
	{
		return mCubemapHandle;
	}
}
