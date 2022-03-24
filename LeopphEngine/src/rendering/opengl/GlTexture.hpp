#pragma once

#include <GL/gl3w.h>

#include <type_traits>


namespace leopph::internal
{
	enum class GlTextureType : int
	{
		T1D = GL_TEXTURE_1D,
		T2D = GL_TEXTURE_2D,
		T3D = GL_TEXTURE_3D,
		Rectangle = GL_TEXTURE_RECTANGLE,
		Buffer = GL_TEXTURE_BUFFER,
		CubeMap = GL_TEXTURE_CUBE_MAP,
		T1DArray = GL_TEXTURE_1D_ARRAY,
		T2DArray = GL_TEXTURE_2D_ARRAY,
		CubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
		T2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
		T2DMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	};


	// Handle to an OpenGL texture object.
	template<GlTextureType TexType>
	class GlTexture
	{
		public:
			GlTexture();

			GlTexture(GlTexture const& other) = delete;
			auto operator=(GlTexture const& other) -> GlTexture& = delete;

			// Sets the other to 0.
			GlTexture(GlTexture&& other) noexcept;

			// Sets the other to 0.
			auto operator=(GlTexture&& other) noexcept -> GlTexture&;

			~GlTexture() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T1D>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T2D>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T3D>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::Rectangle>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::Buffer>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::CubeMap>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T1DArray>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T2DArray>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::CubeMapArray>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T2DMultisample>>);
	static_assert(std::is_default_constructible_v<GlTexture<GlTextureType::T2DMultisampleArray>>);

	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T1D>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T2D>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T3D>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::Rectangle>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::Buffer>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::CubeMap>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T1DArray>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T2DArray>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::CubeMapArray>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T2DMultisample>>);
	static_assert(!std::is_copy_constructible_v<GlTexture<GlTextureType::T2DMultisampleArray>>);

	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T1D>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T2D>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T3D>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::Rectangle>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::Buffer>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::CubeMap>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T1DArray>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T2DArray>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::CubeMapArray>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T2DMultisample>>);
	static_assert(!std::is_copy_assignable_v<GlTexture<GlTextureType::T2DMultisampleArray>>);

	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T1D>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T2D>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T3D>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::Rectangle>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::Buffer>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::CubeMap>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T1DArray>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T2DArray>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::CubeMapArray>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T2DMultisample>>);
	static_assert(std::is_move_constructible_v<GlTexture<GlTextureType::T2DMultisampleArray>>);

	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T1D>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T2D>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T3D>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::Rectangle>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::Buffer>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::CubeMap>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T1DArray>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T2DArray>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::CubeMapArray>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T2DMultisample>>);
	static_assert(std::is_move_assignable_v<GlTexture<GlTextureType::T2DMultisampleArray>>);


	template<GlTextureType TexType>
	GlTexture<TexType>::GlTexture() :
		m_Name{
			[]
			{
				GLuint name;
				glCreateTextures(static_cast<int>(TexType), 1, &name);
				return name;
			}()
		}
	{}


	template<GlTextureType TexType>
	GlTexture<TexType>::GlTexture(GlTexture&& other) noexcept :
		m_Name{other.Name()}
	{
		other.m_Name = 0;
	}


	template<GlTextureType TexType>
	auto GlTexture<TexType>::operator=(GlTexture&& other) noexcept -> GlTexture&
	{
		glDeleteTextures(1, &m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	template<GlTextureType TexType>
	GlTexture<TexType>::~GlTexture() noexcept
	{
		glDeleteTextures(1, &m_Name);
	}


	template<GlTextureType TexType>
	GlTexture<TexType>::operator unsigned() const noexcept
	{
		return m_Name;
	}


	template<GlTextureType TexType>
	auto GlTexture<TexType>::Name() const noexcept -> GLuint
	{
		return m_Name;
	}
}
