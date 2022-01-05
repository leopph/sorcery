#include "CubeShadowMap.hpp"

#include "../config/Settings.hpp"


namespace leopph::internal
{
	CubeShadowMap::CubeShadowMap() :
		m_Framebuffer{},
		m_Cubemap{},
		m_Res{static_cast<GLsizei>(Settings::PointLightShadowMapResolution())}
	{
		glCreateFramebuffers(1, &m_Framebuffer);
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);

		InitCubemap();
	}


	CubeShadowMap::~CubeShadowMap() noexcept
	{
		DeinitCubemap();
		glDeleteFramebuffers(1, &m_Framebuffer);
	}


	auto CubeShadowMap::BindForWritingAndClear() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Res, m_Res);

		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &CLEAR_DEPTH);
	}


	auto CubeShadowMap::BindForReading(ShaderProgram& shader, const std::string_view uniformName, const GLuint texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_Cubemap);
		shader.SetUniform(uniformName, texUnit);
		return texUnit + 1;
	}


	auto CubeShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Res = static_cast<GLsizei>(event.Resolution);
		DeinitCubemap();
		InitCubemap();
	}


	auto CubeShadowMap::InitCubemap() -> void
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Cubemap);
		glTextureStorage2D(m_Cubemap, 1, GL_DEPTH_COMPONENT24, m_Res, m_Res);

		glTextureParameteri(m_Cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_Cubemap, 0);
	}


	auto CubeShadowMap::DeinitCubemap() const -> void
	{
		glDeleteTextures(1, &m_Cubemap);
	}
}
