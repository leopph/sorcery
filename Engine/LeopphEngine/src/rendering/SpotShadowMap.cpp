#include "rendering/SpotShadowMap.hpp"

#include "InternalContext.hpp"
#include "SettingsImpl.hpp"


namespace leopph::internal
{
	SpotShadowMap::SpotShadowMap() :
		m_Res{static_cast<GLsizei>(GetSettingsImpl()->SpotShadowResolution())}
	{
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);

		ConfigureShadowMap();
	}


	auto SpotShadowMap::Clear() const noexcept -> void
	{
		static GLfloat constexpr clear{1};
		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &clear);
	}


	auto SpotShadowMap::BindForWriting() const noexcept -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Res, m_Res);
	}


	auto SpotShadowMap::BindForWritingAndClear() const -> void
	{
		BindForWriting();
		Clear();
	}


	auto SpotShadowMap::BindForReading(ShaderProgram& shader, std::string_view const uniformName, GLuint const texUnit) const -> GLuint
	{
		glBindTextureUnit(texUnit, m_ShadowMap);
		shader.SetUniform(uniformName, static_cast<GLint>(texUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
		return texUnit + 1;
	}


	auto SpotShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Res = static_cast<GLsizei>(event.Resolution);
		ConfigureShadowMap();
	}


	auto SpotShadowMap::ConfigureShadowMap() -> void
	{
		m_ShadowMap = {};
		glTextureStorage2D(m_ShadowMap, 1, GL_DEPTH_COMPONENT24, m_Res, m_Res);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_ShadowMap, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_ShadowMap, 0);
	}
}
