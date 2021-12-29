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


	auto CascadedShadowMap::WorldToClipMatrix(const std::size_t cascadeIndex, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix) const -> Matrix4
	{
		const auto& camera{*Camera::Active()};
		const auto cascadeBounds = CascadeBoundsViewSpace(cascadeIndex);
		const auto cascadeNear{cascadeBounds[0]};
		const auto cascadeFar{cascadeBounds[1]};

		const auto tanHalfHorizFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Horizontal)) / 2.0f)};
		const auto tanHalfVertFov{math::Tan(math::ToRadians(camera.Fov(Camera::FovDirection::Vertical)) / 2.0f)};

		const auto xn{cascadeNear * tanHalfHorizFov};
		const auto xf{cascadeFar * tanHalfHorizFov};
		const auto yn{cascadeNear * tanHalfVertFov};
		const auto yf{cascadeFar * tanHalfVertFov};

		const std::array frustumVertices
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

		const auto camToLightMat{cameraInverseMatrix * lightViewMatrix};

		std::ranges::for_each(frustumVertices, [&](const auto& vertex)
		{
			const auto lightSpaceVertex = vertex * camToLightMat;
			min = Vector3{std::min(min[0], lightSpaceVertex[0]), std::min(min[1], lightSpaceVertex[1]), std::min(min[2], lightSpaceVertex[2])};
			max = Vector3{std::max(max[0], lightSpaceVertex[0]), std::max(max[1], lightSpaceVertex[1]), std::max(max[2], lightSpaceVertex[2])};
		});

		return lightViewMatrix * Matrix4::Ortographic(min[0], max[0], max[1], min[1], 0, max[2]);
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
