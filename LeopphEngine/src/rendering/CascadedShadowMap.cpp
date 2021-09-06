#include "CascadedShadowMap.hpp"

#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../math/LeopphMath.hpp"
#include "../windowing/window.h"

#include <glad/glad.h>


namespace leopph::impl
{
	CascadedShadowMap::CascadedShadowMap() :
		m_Fbo{}, m_TexIds{}, m_TexBindStartIndex{}
	{
		glCreateFramebuffers(1, &m_Fbo);
		Init(Settings::DirectionalShadowMapResolutions());
	}


	CascadedShadowMap::~CascadedShadowMap()
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Fbo);
	}


	void CascadedShadowMap::BindTextureForWriting(std::size_t cascadeIndex) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
		glNamedFramebufferTexture(m_Fbo, GL_DEPTH_ATTACHMENT, m_TexIds.at(cascadeIndex), 0);
		glViewport(0, 0, static_cast<GLsizei>(Settings::DirectionalShadowMapResolutions().at(cascadeIndex)), static_cast<GLsizei>(Settings::DirectionalShadowMapResolutions().at(cascadeIndex)));
		Clear();
	}

	
	void CascadedShadowMap::UnbindTextureFromWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(Window::Get().Width()), static_cast<GLsizei>(Window::Get().Height()));
	}


	void CascadedShadowMap::BindTexturesForReading(std::size_t texUnit)
	{
		m_TexBindStartIndex = texUnit;

		for (const auto& texture : m_TexIds)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(texture));
			++texUnit;
		}
	}


	void CascadedShadowMap::UnbindTexturesFromReading() const
	{
		for (std::size_t texUnit{m_TexBindStartIndex}, i{0}; i < m_TexIds.size(); ++i, ++texUnit)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(0));
		}
	}


	void CascadedShadowMap::Clear() const
	{
		int clearValue{1};
		glClearNamedFramebufferiv(m_Fbo, GL_DEPTH, 0, &clearValue);
	}


	Matrix4 CascadedShadowMap::ProjectionMatrix(std::size_t cascadeIndex) const
	{
		const auto& camera{*Camera::Active()};
		const auto cascadeDepth{camera.FarClipPlane() - camera.NearClipPlane()};
		const auto cascadeNear{cascadeIndex * cascadeDepth};
		const auto cascadeFar{(cascadeIndex + 1) * cascadeDepth};

		const auto tanHalfHorizFov{math::Tan(math::ToRadians(camera.FOV(Camera::FOV_HORIZONTAL)) / 2.0f)};
		const auto tanHalfVertFov{math::Tan(math::ToRadians(camera.FOV(Camera::FOV_VERTICAL)) / 2.0f)};

		const auto xn{cascadeNear * tanHalfHorizFov};
		const auto xf{cascadeFar * tanHalfHorizFov};
		const auto yn{cascadeNear * tanHalfVertFov};
		const auto yf{cascadeFar * tanHalfVertFov};

		// TODO

		return {};
	}


	void CascadedShadowMap::OnEventReceived(const DirShadowMapResChangedEvent& event)
	{
		Deinit();
		Init(event.Resolutions);
	}


	void CascadedShadowMap::Init(std::vector<std::size_t> resolutions)
	{
		m_TexIds.resize(resolutions.size());
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(resolutions.size()), m_TexIds.data());

		for (std::size_t i = 0; i < resolutions.size(); i++)
		{
			glTextureStorage2D(m_TexIds[i], 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(resolutions[i]), static_cast<GLsizei>(resolutions[i]));
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_COMPARE_MODE, GL_NONE);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glNamedFramebufferDrawBuffer(m_Fbo, GL_NONE);
		glNamedFramebufferReadBuffer(m_Fbo, GL_NONE);
	}


	void CascadedShadowMap::Deinit()
	{
		glDeleteTextures(static_cast<GLsizei>(m_TexIds.size()), m_TexIds.data());
	}
}