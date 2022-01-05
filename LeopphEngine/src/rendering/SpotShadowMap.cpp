#include "SpotShadowMap.hpp"

#include "../config/Settings.hpp"


namespace leopph::internal
{
	SpotShadowMap::SpotShadowMap() :
		m_Framebuffer{},
		m_ShadowMap{},
		m_Res{static_cast<GLsizei>(Settings::SpotLightShadowMapResolution())}
	{
		glCreateFramebuffers(1, &m_Framebuffer);
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);

		InitShadowMap();
	}


	SpotShadowMap::~SpotShadowMap() noexcept
	{
		DeinitShadowMap();
		glDeleteFramebuffers(1, &m_Framebuffer);
	}


	auto SpotShadowMap::BindForWritingAndClear() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Res, m_Res);

		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &CLEAR_DEPTH);
	}


	auto SpotShadowMap::BindForReading(ShaderProgram& shader, const std::string_view uniformName, const GLuint texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_ShadowMap);
		shader.SetUniform(uniformName, texUnit);
		return texUnit + 1;
	}


	auto SpotShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Res = static_cast<GLsizei>(event.Resolution);
		DeinitShadowMap();
		InitShadowMap();
	}


	auto SpotShadowMap::InitShadowMap() -> void
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ShadowMap);

		glTextureStorage2D(m_ShadowMap, 1, GL_DEPTH_COMPONENT24, m_Res, m_Res);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_ShadowMap, 0);
	}


	auto SpotShadowMap::DeinitShadowMap() const -> void
	{
		glDeleteTextures(1, &m_ShadowMap);
	}
}
