#include "SpotLightShadowMap.hpp"

#include "../config/Settings.hpp"
#include "../windowing/WindowBase.hpp"

#include <glad/glad.h>


namespace leopph::internal
{
	SpotLightShadowMap::SpotLightShadowMap() :
		Id{m_Fbo},
		m_Fbo{},
		m_DepthTex{},
		m_CurrentBindIndex{},
		m_Resolution{Settings::SpotLightShadowMapResolution()}
	{
		glCreateFramebuffers(1, &m_Fbo);
		glNamedFramebufferDrawBuffer(m_Fbo, GL_NONE);
		glNamedFramebufferReadBuffer(m_Fbo, GL_NONE);

		Init();
	}

	SpotLightShadowMap::SpotLightShadowMap(SpotLightShadowMap&& other) noexcept :
		Id{m_Fbo},
		m_Fbo{other.m_Fbo},
		m_DepthTex{other.m_DepthTex},
		m_CurrentBindIndex{other.m_CurrentBindIndex},
		m_Resolution{other.m_Resolution}
	{
		other.m_Fbo = 0;
		other.m_DepthTex = 0;
	}

	SpotLightShadowMap::~SpotLightShadowMap()
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Fbo);
	}

	auto SpotLightShadowMap::operator=(SpotLightShadowMap&& other) noexcept -> SpotLightShadowMap&
	{
		m_Fbo = other.m_Fbo;
		m_DepthTex = other.m_DepthTex;
		m_CurrentBindIndex = other.m_CurrentBindIndex;
		m_Resolution = other.m_Resolution;

		other.m_Fbo = 0;
		other.m_DepthTex = 0;
		other.m_CurrentBindIndex = 0;

		return *this;
	}

	auto SpotLightShadowMap::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
		glViewport(0, 0, static_cast<GLsizei>(m_Resolution), static_cast<GLsizei>(m_Resolution));
		Clear();
	}

	auto SpotLightShadowMap::UnbindFromWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{WindowBase::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}

	auto SpotLightShadowMap::BindForReading(ShaderProgram& shader, const int textureUnit) const -> int
	{
		m_CurrentBindIndex = textureUnit;
		glBindTextureUnit(static_cast<GLuint>(m_CurrentBindIndex), m_DepthTex);
		shader.SetUniform("u_ShadowMap", textureUnit);
		return m_CurrentBindIndex + 1;
	}

	auto SpotLightShadowMap::UnbindFromReading() const -> void
	{
		glBindTextureUnit(m_CurrentBindIndex, 0);
	}

	auto SpotLightShadowMap::Clear() const -> void
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_Fbo, GL_DEPTH, 0, &clearValue);
	}

	auto SpotLightShadowMap::Init() -> void
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthTex);

		glTextureStorage2D(m_DepthTex, 1, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(m_Resolution), static_cast<GLsizei>(m_Resolution));
		glTextureParameteri(m_DepthTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_DepthTex, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glNamedFramebufferTexture(m_Fbo, GL_DEPTH_ATTACHMENT, m_DepthTex, 0);
	}

	auto SpotLightShadowMap::Deinit() const -> void
	{
		glDeleteTextures(1, &m_DepthTex);
	}

	auto SpotLightShadowMap::OnEventReceived(EventParamType event) -> void
	{
		m_Resolution = event.Resolution;
		Deinit();
		Init();
	}
}
