#include "Texture2D.hpp"

#include "Context.hpp"
#include "GlCore.hpp"
#include "Logger.hpp"
#include "Renderer.hpp"

#include <format>


namespace leopph
{
	Texture2D::Texture2D(Image const& img) :
		mWidth{img.get_width()},
		mHeight{img.get_height()}
	{
		GLenum colorFormat, internalFormat;

		switch (img.get_num_channels())
		{
			case 1:
			{
				colorFormat = GL_RED;
				internalFormat = GL_R8;
				break;
			}

			case 3:
			{
				colorFormat = GL_RGB;

				if (img.get_encoding() == ColorEncoding::sRGB)
				{
					internalFormat = GL_SRGB8;
				}
				else
				{
					internalFormat = GL_RGB8;
				}
				break;
			}

			case 4:
			{
				colorFormat = GL_RGBA;

				if (img.get_encoding() == ColorEncoding::sRGB)
				{
					internalFormat = GL_SRGB8_ALPHA8;
				}
				else
				{
					internalFormat = GL_RGBA8;
				}
				break;
			}

			default:
				internal::Logger::Instance().Error(std::format("Failed to create texture: input image channel count [{}] is not supported.", img.get_num_channels()));
				return;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &mTexture);
		glTextureStorage2D(mTexture, 1, internalFormat, mWidth, mHeight);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // This is a permanent change because of the tightly packed image data. Unnecessary to set it every time.
		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, colorFormat, GL_UNSIGNED_BYTE, img.get_data().data());

		glGenerateTextureMipmap(mTexture);

		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		mHandle = glGetTextureHandleARB(mTexture);
		glMakeTextureHandleResidentARB(mHandle);

		internal::get_renderer()->register_texture_2d(shared_from_this());
	}



	Texture2D::~Texture2D()
	{
		internal::get_renderer()->unregister_texture_2d(shared_from_this());
		glDeleteTextures(1, &mTexture);
	}



	u32 Texture2D::get_handle() const
	{
		return mTexture;
	}



	u32 Texture2D::get_width() const
	{
		return mWidth;
	}



	u32 Texture2D::get_height() const
	{
		return mHeight;
	}
}
