#include "CascadedShadowMap.hpp"

#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../math/LeopphMath.hpp"
#include "../windowing/WindowBase.hpp"

#include <algorithm>
#include <array>
#include <limits>


namespace leopph::internal
{
	CascadedShadowMap::CascadedShadowMap() :
		m_Framebuffer{},
		m_TexBindStartIndex{}
	{
		glCreateFramebuffers(1, &m_Framebuffer);
		Init(Settings::DirShadowCascades());
	}


	CascadedShadowMap::~CascadedShadowMap() noexcept
	{
		Deinit();
		glDeleteFramebuffers(1, &m_Framebuffer);
	}


	auto CascadedShadowMap::BindForWriting(const std::size_t cascadeIndex) const -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_Textures.at(cascadeIndex), 0);
		const auto res{static_cast<GLsizei>(Settings::DirShadowCascades()[cascadeIndex].Resolution)};
		glViewport(0, 0, res, res);
		Clear();
	}


	auto CascadedShadowMap::UnbindFromWriting() -> void
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(WindowBase::Get().Width()), static_cast<GLsizei>(WindowBase::Get().Height()));
	}


	auto CascadedShadowMap::BindForReading(ShaderProgram& shader, int texUnit) -> int
	{
		m_TexBindStartIndex = texUnit;

		static std::vector<int> texUnits;
		texUnits.clear();

		for (const auto& texture : m_Textures)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(texture));
			texUnits.push_back(texUnit);
			++texUnit;
		}

		shader.SetUniform("u_DirLightShadowMaps", texUnits);
		return texUnit;
	}


	auto CascadedShadowMap::UnbindFromReading() const -> void
	{
		for (auto texUnit{m_TexBindStartIndex}, i{0}; i < m_Textures.size(); ++i, ++texUnit)
		{
			glBindTextureUnit(static_cast<GLuint>(texUnit), static_cast<GLuint>(0));
		}
	}


	auto CascadedShadowMap::Clear() const -> void
	{
		constexpr float clearValue{1};
		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &clearValue);
	}


	auto CascadedShadowMap::CascadeMatrix(const std::size_t cascadeIndex, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix) const -> Matrix4
	{
		const auto& camera{*Camera::Active()};
		const auto cascadeBounds = CascadeBoundsViewSpace(cascadeIndex);
		const auto cascadeNear{cascadeBounds[0]};
		const auto cascadeFar{cascadeBounds[1]};

		// Calculate the cascade's vertices in camera space.

		const auto tanHalfHorizFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Horizontal)) / 2.0f)};
		const auto tanHalfVertFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Vertical)) / 2.0f)};

		const auto xn{cascadeNear * tanHalfHorizFov};
		const auto xf{cascadeFar * tanHalfHorizFov};
		const auto yn{cascadeNear * tanHalfVertFov};
		const auto yf{cascadeFar * tanHalfVertFov};

		// The cascade vertices in camera space.
		const std::array cascadeVertsCam
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

		// The light space mininum point of the bounding box of the cascade
		Vector3 bBoxMinLight{std::numeric_limits<float>::max()};
		// The light space maximum point of the bounding box of the cascade
		Vector3 bBoxMaxLight{std::numeric_limits<float>::min()};

		// Transform from camera space to light space
		const auto camToLightMat{cameraInverseMatrix * lightViewMatrix};

		// Calculate the bounding box min and max points by transforming the vertices to light space.
		std::ranges::for_each(cascadeVertsCam, [&](const auto& vertex)
		{
			const auto vertLight = vertex * camToLightMat;
			bBoxMinLight = Vector3{std::min(bBoxMinLight[0], vertLight[0]), std::min(bBoxMinLight[1], vertLight[1]), std::min(bBoxMinLight[2], vertLight[2])};
			bBoxMaxLight = Vector3{std::max(bBoxMaxLight[0], vertLight[0]), std::max(bBoxMaxLight[1], vertLight[1]), std::max(bBoxMaxLight[2], vertLight[2])};
		});

		// The projection matrix that uses the calculated min/max values of the bounding box. Essentially THE bounding box.
		const auto lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2], bBoxMaxLight[2])};

		// Crop matrix scale factor
		const Vector2 cropScale{2.f / (bBoxMaxLight[0] - bBoxMinLight[0]), 2.f / (bBoxMaxLight[1] - bBoxMinLight[1])};
		// Crop matrix offset
		const Vector2 cropOffset{-0.5f * (bBoxMaxLight[0] + bBoxMinLight[0]) * cropScale[0], -0.5f * (bBoxMaxLight[1] + bBoxMinLight[1]) * cropScale[1]};
		// Crop matrix
		const Matrix4 cropMat{
			cropScale[0], 0, 0, 0,
			0, cropScale[1], 0, 0,
			0, 0, 1, 0,
			cropOffset[0], cropOffset[1], 0, 1
		};

		return lightViewMatrix * lightProjMat * cropMat;
	}


	auto CascadedShadowMap::CascadeBoundsViewSpace(const std::size_t cascadeIndex) const -> Vector2
	{
		const auto& cascades{Settings::DirShadowCascades()};
		const auto cascadeNear{cascadeIndex > 0 ? cascades[cascadeIndex - 1].Bound : 0};
		const auto cascadeFar{cascades[cascadeIndex].Bound};
		return Vector2{cascadeNear, cascadeFar};
	}


	auto CascadedShadowMap::OnEventReceived(const DirCascadeChangeEvent& event) -> void
	{
		Deinit();
		Init(event.Cascades);
	}


	auto CascadedShadowMap::Init(const std::span<const ShadowCascade> cascades) -> void
	{
		m_Textures.resize(cascades.size());
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(cascades.size()), m_Textures.data());

		for (std::size_t i = 0; i < cascades.size(); i++)
		{
			glTextureStorage2D(m_Textures[i], 1, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(cascades[i].Resolution), static_cast<GLsizei>(cascades[i].Resolution));
			glTextureParameteri(m_Textures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_Textures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_Textures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_Textures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_Textures[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(m_Textures[i], GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}

		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_Textures.front(), 0);
	}


	auto CascadedShadowMap::Deinit() const -> void
	{
		glDeleteTextures(static_cast<GLsizei>(m_Textures.size()), m_Textures.data());
	}
}
