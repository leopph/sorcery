#include "rendering/CubeShadowMap.hpp"

#include "InternalContext.hpp"
#include "SettingsImpl.hpp"

#include <limits>


namespace leopph::internal
{
	CubeShadowMap::CubeShadowMap() :
		m_Res{static_cast<GLsizei>(GetSettingsImpl()->PointShadowResolution())}
	{
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);

		ConfigCubeMap();
	}


	auto CubeShadowMap::Clear() const noexcept -> void
	{
		static GLfloat constexpr clearColor[]
		{
			std::numeric_limits<GLfloat>::max(),
			std::numeric_limits<GLfloat>::max(),
			std::numeric_limits<GLfloat>::max(),
			std::numeric_limits<GLfloat>::max()
		};
		static GLfloat constexpr clearDepth{1};
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &clearDepth);
	}


	auto CubeShadowMap::BindForWriting(GLint const face) const noexcept -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glNamedFramebufferTextureLayer(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_Cubemap, 0, face);
		glViewport(0, 0, m_Res, m_Res);
	}


	auto CubeShadowMap::BindForWritingAndClear(GLint const face) const -> void
	{
		BindForWriting(face);
		Clear();
	}


	auto CubeShadowMap::BindForReading(ShaderProgram& shader, std::string_view const uniformName, GLuint const texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_Cubemap);
		shader.SetUniform(uniformName, static_cast<GLint>(texUnit)); /* cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */
		return texUnit + 1;
	}


	auto CubeShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Res = static_cast<GLsizei>(event.Resolution);
		ConfigCubeMap();
	}


	auto CubeShadowMap::ConfigCubeMap() -> void
	{
		m_Cubemap = {};
		glTextureStorage2D(m_Cubemap, 1, GL_R32F, m_Res, m_Res);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		m_DepthBuffer = {};
		glNamedRenderbufferStorage(m_DepthBuffer, GL_DEPTH_COMPONENT24, m_Res, m_Res);
		glNamedFramebufferRenderbuffer(m_Framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);
	}
}
