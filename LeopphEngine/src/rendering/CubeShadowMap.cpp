#include "CubeShadowMap.hpp"

#include "../config/Settings.hpp"
#include "../windowing/WindowBase.hpp"

#include <glad/glad.h>


namespace leopph::internal
{
	CubeShadowMap::CubeShadowMap() :
		m_FrameBufferName{},
		m_CubeMapName{},
		m_BoundTexUnit{0}
	{
		glCreateFramebuffers(1, &m_FrameBufferName);
		Init(Settings::PointLightShadowMapResolution());
	}

	CubeShadowMap::~CubeShadowMap()
	{
		Deinit();
		glDeleteFramebuffers(1, &m_FrameBufferName);
	}

	auto CubeShadowMap::OnEventReceived(EventParamType event) -> void
	{
		Deinit();
		Init(event.Resolution);
	}

	auto CubeShadowMap::Init(const std::size_t resolution) -> void
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_CubeMapName);
		glTextureStorage2D(m_CubeMapName, 1, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution));

		glTextureParameteri(m_CubeMapName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_CubeMapName, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		glNamedFramebufferDrawBuffer(m_FrameBufferName, GL_NONE);
		glNamedFramebufferReadBuffer(m_FrameBufferName, GL_NONE);
		glNamedFramebufferTexture(m_FrameBufferName, GL_DEPTH_ATTACHMENT, m_CubeMapName, 0);
	}

	auto CubeShadowMap::Deinit() const -> void
	{
		glDeleteTextures(1, &m_CubeMapName);
	}

	auto CubeShadowMap::Clear() const -> void
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_FrameBufferName, GL_DEPTH, 0, &clearValue);
	}

	auto CubeShadowMap::BindForWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferName);
		glViewport(0, 0, static_cast<GLsizei>(Settings::PointLightShadowMapResolution()), static_cast<GLsizei>(Settings::PointLightShadowMapResolution()));
	}

	auto CubeShadowMap::UnbindFromWriting() const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{WindowBase::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}

	auto CubeShadowMap::BindForReading(ShaderProgram& shader, const int texUnit) -> int
	{
		m_BoundTexUnit = texUnit;
		glBindTextureUnit(m_BoundTexUnit, m_CubeMapName);
		shader.SetUniform("u_ShadowMap", m_BoundTexUnit);
		return texUnit + 1;
	}

	auto CubeShadowMap::UnbindFromReading() const -> void
	{
		glBindTextureUnit(m_BoundTexUnit, 0);
	}
}
