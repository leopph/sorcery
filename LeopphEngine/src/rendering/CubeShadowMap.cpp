#include "CubeShadowMap.hpp"

#include "../config/Settings.hpp"
#include "../windowing/Window.hpp"

#include <glad/glad.h>


namespace leopph::impl
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


	void CubeShadowMap::OnEventReceived(EventParamType event)
	{
		Deinit();
		Init(event.Resolution);
	}


	void CubeShadowMap::Init(const std::size_t resolution)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_CubeMapName);
		glTextureStorage2D(m_CubeMapName, 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution));

		/*for (std::size_t i = 0; i < 6; i++)
		{
			glTextureSubImage3D(m_CubeMapName, 0, 0, 0, static_cast<GLsizei>(i), static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution), 1, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}*/

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


	void CubeShadowMap::Deinit() const
	{
		glDeleteTextures(1, &m_CubeMapName);
	}


	void CubeShadowMap::Clear() const
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_FrameBufferName, GL_DEPTH, 0, &clearValue);
	}
	

	void CubeShadowMap::BindForWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferName);
		glViewport(0, 0, static_cast<GLsizei>(Settings::PointLightShadowMapResolution()), static_cast<GLsizei>(Settings::PointLightShadowMapResolution()));
	}


	void CubeShadowMap::UnbindFromWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const auto& window{Window::Get()};
		glViewport(0, 0, static_cast<GLsizei>(window.Width()), static_cast<GLsizei>(window.Height()));
	}


	int CubeShadowMap::BindForReading(ShaderProgram& shader, int texUnit)
	{
		m_BoundTexUnit = texUnit;
		glBindTextureUnit(m_BoundTexUnit, m_CubeMapName);
		shader.SetUniform("u_ShadowMap", m_BoundTexUnit);
		return texUnit + 1;
	}


	void CubeShadowMap::UnbindFromReading() const
	{
		glBindTextureUnit(m_BoundTexUnit, 0);
	}

}