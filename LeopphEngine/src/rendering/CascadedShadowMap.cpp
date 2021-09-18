#include "CascadedShadowMap.hpp"

#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../math/LeopphMath.hpp"
#include "../windowing/Window.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <limits>


namespace leopph::impl
{
	CascadedShadowMap::CascadedShadowMap() :
		m_Fbo{}, m_TexBindStartIndex{}
	{
		glCreateFramebuffers(1, &m_Fbo);
		Init(Settings::DirectionalShadowMapResolutions());
	}


	CascadedShadowMap::~CascadedShadowMap()
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Fbo);
	}


	void CascadedShadowMap::BindForWriting(const std::size_t cascadeIndex) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
		glNamedFramebufferTexture(m_Fbo, GL_DEPTH_ATTACHMENT, m_TexIds.at(cascadeIndex), 0);
		glViewport(0, 0, static_cast<GLsizei>(Settings::DirectionalShadowMapResolutions().at(cascadeIndex)), static_cast<GLsizei>(Settings::DirectionalShadowMapResolutions().at(cascadeIndex)));
		Clear();
	}

	
	void CascadedShadowMap::UnbindFromWriting() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(Window::Get().Width()), static_cast<GLsizei>(Window::Get().Height()));
	}


	int CascadedShadowMap::BindForReading(const DefDirShader& shader, int texUnit)
	{
		m_TexBindStartIndex = texUnit;

		static std::vector<int> texUnits;
		texUnits.clear();

		for (const auto& texture : m_TexIds)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(texture));
			texUnits.push_back(texUnit);
			++texUnit;
		}

		shader.SetShadowMaps(texUnits);
		return texUnit;
	}


	void CascadedShadowMap::UnbindFromReading() const
	{
		for (auto texUnit{m_TexBindStartIndex}, i{0}; i < m_TexIds.size(); ++i, ++texUnit)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(0));
		}
	}


	void CascadedShadowMap::Clear() const
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_Fbo, GL_DEPTH, 0, &clearValue);
	}


	Matrix4 CascadedShadowMap::WorldToClipMatrix(const std::size_t cascadeIndex, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix) const
	{
		const auto& camera{*Camera::Active()};
		const auto cascadeDepth{(camera.FarClipPlane() - camera.NearClipPlane()) / m_TexIds.size()};
		const auto cascadeNear{camera.NearClipPlane() + cascadeIndex * cascadeDepth};
		const auto cascadeFar{camera.NearClipPlane() + (cascadeIndex + 1) * cascadeDepth};

		const auto tanHalfHorizFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Horizontal)) / 2.0f)};
		const auto tanHalfVertFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Vertical)) / 2.0f)};

		const auto xn{cascadeNear * tanHalfHorizFov};
		const auto xf{cascadeFar * tanHalfHorizFov};
		const auto yn{cascadeNear * tanHalfVertFov};
		const auto yf{cascadeFar * tanHalfVertFov};

		std::array frustumVertices
		{
			Vector4{-xn, -yn, cascadeNear, 1.f},
			Vector4{xn, -yn, cascadeNear, 1.f},
			Vector4{xn, yn, cascadeNear, 1.f},
			Vector4{-xn, yn, cascadeNear, 1.f},
			Vector4{-xf, -yf, cascadeFar, 1.f},
			Vector4{xf, -yf, cascadeFar, 1.f},
			Vector4{xf, yf, cascadeFar, 1.f},
			Vector4{-xf, yf, cascadeFar, 1.f},
		};

		Vector3 min{std::numeric_limits<float>::max()};
		Vector3 max{std::numeric_limits<float>::min()};

		std::ranges::for_each(frustumVertices.begin(), frustumVertices.end(), [&](const auto& vertex)
		{
			const auto lightSpaceVertex = vertex * cameraInverseMatrix * lightViewMatrix;
			min = Vector3{std::min(min[0], lightSpaceVertex[0]), std::min(min[1], lightSpaceVertex[1]), std::min(min[2], lightSpaceVertex[2])};
			max = Vector3{std::max(max[0], lightSpaceVertex[0]), std::max(max[1], lightSpaceVertex[1]), std::max(max[2], lightSpaceVertex[2])};
		});

		return lightViewMatrix * Matrix4::Ortographic(min[0], max[0], max[1], min[1], 0, max[2]);
	}


	Vector2 CascadedShadowMap::CascadeBoundsViewSpace(const std::size_t cascadeIndex) const
	{
		const auto& camera{*Camera::Active()};
		const auto cascadeDepth{(camera.FarClipPlane() - camera.NearClipPlane()) / static_cast<float>(m_TexIds.size())};
		return Vector2
		{
			camera.NearClipPlane() + static_cast<float>(cascadeIndex) * cascadeDepth,
			camera.NearClipPlane() + static_cast<float>(cascadeIndex + 1) * cascadeDepth
		};
	}



	void CascadedShadowMap::OnEventReceived(const DirShadowMapResChangedEvent& event)
	{
		Deinit();
		Init(event.Resolutions);
	}


	void CascadedShadowMap::Init(const std::vector<std::size_t>& resolutions)
	{
		m_TexIds.resize(resolutions.size());
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(resolutions.size()), m_TexIds.data());

		for (std::size_t i = 0; i < resolutions.size(); i++)
		{
			glTextureStorage2D(m_TexIds[i], 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(resolutions[i]), static_cast<GLsizei>(resolutions[i]));
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(m_TexIds[i], GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}

		glNamedFramebufferDrawBuffer(m_Fbo, GL_NONE);
		glNamedFramebufferReadBuffer(m_Fbo, GL_NONE);
		glNamedFramebufferTexture(m_Fbo, GL_DEPTH_ATTACHMENT, m_TexIds.front(), 0);
	}


	void CascadedShadowMap::Deinit()
	{
		glDeleteTextures(static_cast<GLsizei>(m_TexIds.size()), m_TexIds.data());
	}
}