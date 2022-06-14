#include "CascadedShadowMap.hpp"

#include "../config/Settings.hpp"
#include "../math/Math.hpp"
#include "../windowing/WindowImpl.hpp"

#include <algorithm>
#include <array>
#include <limits>


namespace leopph::internal
{
	CascadedShadowMap::CascadedShadowMap()
	{
		glNamedFramebufferReadBuffer(m_Framebuffer, GL_NONE);
		glNamedFramebufferDrawBuffer(m_Framebuffer, GL_NONE);

		ConfigCascades(Settings::Instance().DirShadowResolution());
	}


	auto CascadedShadowMap::Clear() const -> void
	{
		static GLfloat constexpr clear{1};
		glClearNamedFramebufferfv(m_Framebuffer, GL_DEPTH, 0, &clear);
	}


	auto CascadedShadowMap::BindForWriting(std::size_t const cascadeIndex) const -> void
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);
		auto const& [res, shadowMap] = m_Cascades.at(cascadeIndex);
		glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_ATTACHMENT, shadowMap, 0);
		glViewport(0, 0, static_cast<GLsizei>(res), static_cast<GLsizei>(res));
	}


	auto CascadedShadowMap::BindForReading(ShaderProgram& shader, std::string_view const uniformName, GLuint texUnit) const -> GLuint
	{
		static std::vector<int> texUnits;
		texUnits.clear();

		for (auto const& [res, shadowMap] : m_Cascades)
		{
			glBindTextureUnit(texUnit, shadowMap);
			texUnits.push_back(static_cast<GLint>(texUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			++texUnit;
		}

		shader.SetUniform(uniformName, texUnits);
		return texUnit;
	}


	auto CascadedShadowMap::CascadeMatrix(Frustum const& frustum, std::span<CascadeBounds const> cascadeBounds, Matrix4 const& worldTolightMat, Matrix4 const& camToLightMat, float const bBoxNearOffset) -> std::span<Matrix4>
	{
		// Distance from the frustums near face to its far face.
		auto const frustumDepth = frustum.FarBottomLeft[2] - frustum.NearBottomLeft[2];

		static std::vector<Matrix4> matrices;
		matrices.clear();

		for (auto const& [near, far] : cascadeBounds)
		{
			// The normalized distance of the cascades faces from the near frustum face.

			auto const cascadeNearDist = (near - frustum.NearBottomLeft[2]) / frustumDepth;
			auto const cascadeFarDist = (far - frustum.NearBottomLeft[2]) / frustumDepth;

			// The cascade vertices in camera view space.
			std::array const cascadeVertsCam
			{
				Vector4{math::Lerp(frustum.NearTopLeft, frustum.FarTopLeft, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frustum.NearBottomLeft, frustum.FarBottomLeft, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frustum.NearBottomRight, frustum.FarBottomRight, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frustum.NearTopRight, frustum.FarTopRight, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frustum.NearTopLeft, frustum.FarTopLeft, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frustum.NearBottomLeft, frustum.FarBottomLeft, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frustum.NearBottomRight, frustum.FarBottomRight, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frustum.NearTopRight, frustum.FarTopRight, cascadeFarDist), 1.f},
			};

			// The light view space mininum point of the bounding box of the cascade
			Vector3 bBoxMinLight{std::numeric_limits<float>::max()};

			// The light view space maximum point of the bounding box of the cascade
			Vector3 bBoxMaxLight{std::numeric_limits<float>::min()};

			// Calculate the bounding box min and max points by transforming the vertices to light space.
			std::ranges::for_each(cascadeVertsCam, [&](auto const& vertex)
			{
				auto const vertLight = vertex * camToLightMat;
				bBoxMinLight = Vector3{std::min(bBoxMinLight[0], vertLight[0]), std::min(bBoxMinLight[1], vertLight[1]), std::min(bBoxMinLight[2], vertLight[2])};
				bBoxMaxLight = Vector3{std::max(bBoxMaxLight[0], vertLight[0]), std::max(bBoxMaxLight[1], vertLight[1]), std::max(bBoxMaxLight[2], vertLight[2])};
			});

			// The projection matrix that uses the calculated min/max values of the bounding box. Essentially THE bounding box + the near clip offset of the DirectionalLight.
			auto const lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2] - bBoxNearOffset, bBoxMaxLight[2])};

			matrices.push_back(worldTolightMat * lightProjMat);
		}

		return matrices;
	}


	auto CascadedShadowMap::CalculateCascadeBounds(float const near, float const far) -> std::span<CascadeBounds>
	{
		auto const& settings{Settings::Instance()};
		auto const numCascades{settings.DirShadowCascadeCount()};
		auto const lambda{settings.DirShadowCascadeCorrection()};
		auto const clipRatio{far / near};

		// On bound borders the far plane is multiplied by this value to avoid precision problems.
		constexpr auto nearFarMult{1.005f};

		static std::vector<CascadeBounds> bounds;
		bounds.resize(numCascades);

		bounds[0].Near = near;
		for (auto i{1ull}; i < numCascades; ++i)
		{
			auto const indRatio{static_cast<float>(i) / static_cast<float>(numCascades)};
			bounds[i].Near = lambda * near * leopph::math::Pow(clipRatio, indRatio) + (1 - lambda) * (near + indRatio * (far - near));
			bounds[i - 1].Far = bounds[i].Near * nearFarMult;
		}
		bounds[numCascades - 1].Far = far;

		return bounds;
	}


	auto CascadedShadowMap::OnEventReceived(DirShadowEvent const& event) -> void
	{
		ConfigCascades(event.Resolutions);
	}


	auto CascadedShadowMap::ConfigCascades(std::span<std::size_t const> const resolutions) -> void
	{
		m_Cascades.clear();
		m_Cascades.reserve(resolutions.size());

		for (auto const res : resolutions)
		{
			auto& [resolution, shadowMap] = m_Cascades.emplace_back(res);
			glTextureStorage2D(shadowMap, 1, GL_DEPTH_COMPONENT32, static_cast<GLsizei>(res), static_cast<GLsizei>(res));
			glTextureParameteri(shadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(shadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(shadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(shadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(shadowMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(shadowMap, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}
	}
}
