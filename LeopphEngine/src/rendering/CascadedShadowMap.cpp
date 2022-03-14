#include "CascadedShadowMap.hpp"

#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../math/LeopphMath.hpp"
#include "../windowing/WindowImpl.hpp"

#include <algorithm>
#include <array>
#include <limits>


namespace leopph::internal
{
	CascadedShadowMap::CascadedShadowMap() :
		m_Framebuffer{}
	{
		glCreateFramebuffers(1, &m_Framebuffer);
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);

		InitShadowMaps(Settings::Instance().DirShadowResolution());
	}


	CascadedShadowMap::~CascadedShadowMap() noexcept
	{
		DeinitShadowMaps();
		glDeleteFramebuffers(1, &m_Framebuffer);
	}


	auto CascadedShadowMap::BindForWritingAndClear(const std::size_t cascadeIndex) const -> void
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_ShadowMaps.at(cascadeIndex), 0);
		const auto res{static_cast<GLsizei>(Settings::Instance().DirShadowResolution()[cascadeIndex])};
		glViewport(0, 0, res, res);

		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &CLEAR_DEPTH);
	}


	auto CascadedShadowMap::BindForReading(ShaderProgram& shader, const std::string_view uniformName, GLuint texUnit) const -> GLuint
	{
		static std::vector<int> texUnits;
		texUnits.clear();

		for (const auto texture : m_ShadowMaps)
		{
			glBindTextureUnit(texUnit, texture);
			texUnits.push_back(static_cast<GLint>(texUnit)); /* cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */
			++texUnit;
		}

		shader.SetUniform(uniformName, texUnits);
		return texUnit;
	}


	auto CascadedShadowMap::CascadeMatrix(const CascadeBounds cascadeBounds, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix, const float bBoxNearOffset) const -> Matrix4
	{
		const auto& camera{*Camera::Current()};

		// Calculate the cascade's vertices in camera view space.

		const auto tanHalfHorizFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Horizontal)) / 2.0f)};
		const auto tanHalfVertFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Vertical)) / 2.0f)};

		const auto xn{cascadeBounds.Near * tanHalfHorizFov};
		const auto xf{cascadeBounds.Far * tanHalfHorizFov};
		const auto yn{cascadeBounds.Near * tanHalfVertFov};
		const auto yf{cascadeBounds.Far * tanHalfVertFov};

		// The cascade vertices in camera view space.
		const std::array cascadeVertsCam
		{
			Vector4{-xn, -yn, cascadeBounds.Near, 1.f},
			Vector4{xn, -yn, cascadeBounds.Near, 1.f},
			Vector4{xn, yn, cascadeBounds.Near, 1.f},
			Vector4{-xn, yn, cascadeBounds.Near, 1.f},
			Vector4{-xf, -yf, cascadeBounds.Far, 1.f},
			Vector4{xf, -yf, cascadeBounds.Far, 1.f},
			Vector4{xf, yf, cascadeBounds.Far, 1.f},
			Vector4{-xf, yf, cascadeBounds.Far, 1.f},
		};

		// The light view space mininum point of the bounding box of the cascade
		Vector3 bBoxMinLight{std::numeric_limits<float>::max()};
		// The light view space maximum point of the bounding box of the cascade
		Vector3 bBoxMaxLight{std::numeric_limits<float>::min()};

		// Transform from camera view space to light view space
		const auto camToLightMat{cameraInverseMatrix * lightViewMatrix};

		// Calculate the bounding box min and max points by transforming the vertices to light space.
		std::ranges::for_each(cascadeVertsCam, [&](const auto& vertex)
		{
			const auto vertLight{vertex * camToLightMat};
			bBoxMinLight = Vector3{std::min(bBoxMinLight[0], vertLight[0]), std::min(bBoxMinLight[1], vertLight[1]), std::min(bBoxMinLight[2], vertLight[2])};
			bBoxMaxLight = Vector3{std::max(bBoxMaxLight[0], vertLight[0]), std::max(bBoxMaxLight[1], vertLight[1]), std::max(bBoxMaxLight[2], vertLight[2])};
		});

		// The projection matrix that uses the calculated min/max values of the bounding box. Essentially THE bounding box + the near clip offset of the DirectionalLight.
		const auto lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2] - bBoxNearOffset, bBoxMaxLight[2])};

		return lightViewMatrix * lightProjMat;
	}


	auto CascadedShadowMap::CalculateCascadeBounds(const Camera& cam) const -> std::span<CascadeBounds>
	{
		const auto nearClip{cam.NearClipPlane()};
		const auto farClip{cam.FarClipPlane()};
		const auto& settings{Settings::Instance()};
		const auto numCascades{settings.DirShadowCascadeCount()};
		const auto lambda{settings.DirShadowCascadeCorrection()};
		const auto clipRatio{farClip / nearClip};

		// On bound borders the far plane is multiplied by this value to avoid precision problems.
		constexpr auto nearFarMult{1.005f};

		static std::vector<CascadeBounds> bounds;
		bounds.resize(numCascades);

		bounds[0].Near = nearClip;
		for (auto i{1ull}; i < numCascades; ++i)
		{
			const auto indRatio{static_cast<float>(i) / static_cast<float>(numCascades)};
			bounds[i].Near = lambda * nearClip * math::Pow(clipRatio, indRatio) + (1 - lambda) * (nearClip + indRatio * (farClip - nearClip));
			bounds[i - 1].Far = bounds[i].Near * nearFarMult;
		}
		bounds[numCascades - 1].Far = farClip;

		return bounds;
	}


	auto CascadedShadowMap::OnEventReceived(const DirShadowEvent& event) -> void
	{
		DeinitShadowMaps();
		InitShadowMaps(event.Resolutions);
	}


	auto CascadedShadowMap::InitShadowMaps(const std::span<const std::size_t> ress) -> void
	{
		m_ShadowMaps.resize(ress.size());
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(ress.size()), m_ShadowMaps.data());

		for (std::size_t i = 0; i < ress.size(); i++)
		{
			glTextureStorage2D(m_ShadowMaps[i], 1, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(ress[i]), static_cast<GLsizei>(ress[i]));
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(m_ShadowMaps[i], GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}

		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, m_ShadowMaps.front(), 0);
	}


	auto CascadedShadowMap::DeinitShadowMaps() const -> void
	{
		glDeleteTextures(static_cast<GLsizei>(m_ShadowMaps.size()), m_ShadowMaps.data());
	}
}
