#include "CubeShadowMap.hpp"

#include "../config/Settings.hpp"


namespace leopph::internal
{
	CubeShadowMap::CubeShadowMap() :
		m_Framebuffer{},
		m_Cubemap{},
		m_DepthBuffer{},
		m_Res{static_cast<GLsizei>(Settings::Instance().PointLightShadowMapResolution())}
	{
		glCreateFramebuffers(1, &m_Framebuffer);
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);

		Init();
	}


	CubeShadowMap::~CubeShadowMap() noexcept
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Framebuffer);
	}


	auto CubeShadowMap::BindForWritingAndClear(const GLint face) const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Res, m_Res);

		glNamedFramebufferTextureLayer(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_Cubemap, 0, face);
		glClearNamedFramebufferfv(m_Framebuffer, GL_COLOR, 0, CLEAR_COLOR);
		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &CLEAR_DEPTH);
	}


	auto CubeShadowMap::BindForReading(ShaderProgram& shader, const std::string_view uniformName, const GLuint texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_Cubemap);
		shader.SetUniform(uniformName, static_cast<GLint>(texUnit)); /* cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */
		return texUnit + 1;
	}


	auto CubeShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Res = static_cast<GLsizei>(event.Resolution);
		Deinit();
		Init();
	}


	auto CubeShadowMap::Init() -> void
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthBuffer);
		glTextureStorage2D(m_DepthBuffer, 1, GL_DEPTH_COMPONENT32, m_Res, m_Res);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Cubemap);
		glTextureStorage2D(m_Cubemap, 1, GL_R32F, m_Res, m_Res);

		glTextureParameteri(m_Cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_Cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_DepthBuffer, 0);
	}


	auto CubeShadowMap::Deinit() const -> void
	{
		glDeleteTextures(1, &m_Cubemap);
		glDeleteTextures(1, &m_DepthBuffer);
	}
}
