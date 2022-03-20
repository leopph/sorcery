#include "GlDeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"

#include <algorithm>
#include <array>


namespace leopph::internal
{
	GlDeferredRenderer::GlDeferredRenderer() :
		m_ShadowShader{
			{
				{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
			}
		},
		m_CubeShadowShader{
			{
				{ShaderFamily::CubeShadowMapVertSrc, ShaderType::Vertex},
				{ShaderFamily::CubeShadowMapFragSrc, ShaderType::Fragment}
			}
		},
		m_GeometryShader{
			{
				{ShaderFamily::GeometryPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::GeometryPassFragSrc, ShaderType::Fragment}
			}
		},
		m_LightShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::LightPassFragSrc, ShaderType::Fragment}
			}
		},
		m_SkyboxShader{
			{
				{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
				{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
			}
		}
	{
		glEnable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}


	auto GlDeferredRenderer::Render() -> void
	{
		/* We don't render if there is no camera to use */
		if (Camera::Current() == nullptr)
		{
			return;
		}

		static std::vector<RenderableData> renderables;
		static std::vector<SpotLight const*> spotLights;
		static std::vector<PointLight const*> pointLights;

		renderables.clear();
		CollectRenderables(renderables);
		spotLights.clear();
		CollectSpotLights(spotLights);
		pointLights.clear();
		CollectPointLights(pointLights);

		auto const camViewMat{Camera::Current()->ViewMatrix()};
		auto const camProjMat{Camera::Current()->ProjectionMatrix()};

		RenderGeometry(camViewMat, camProjMat, renderables);
		RenderLights(camViewMat, camProjMat, renderables, spotLights, pointLights);
		RenderSkybox(camViewMat, camProjMat);
		m_ScreenRenderBuffer.CopyColorToDefaultFramebuffer();
	}


	auto GlDeferredRenderer::RenderGeometry(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderableData> const& renderables) -> void
	{
		glStencilFunc(GL_ALWAYS, STENCIL_REF, STENCIL_AND_MASK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glEnable(GL_DEPTH_TEST);

		m_GBuffer.BindForWritingAndClear();

		auto& shader{m_GeometryShader.GetPermutation()};
		shader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		shader.Use();

		for (auto const& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(shader, 0, false);
		}
	}


	auto GlDeferredRenderer::RenderLights(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::span<RenderableData const> const renderables, std::span<SpotLight const*> const spotLights, std::span<PointLight const*> const pointLights) -> void
	{
		auto const dirLight{DataManager::Instance().DirectionalLight()};
		auto const [dirShadow, spotShadows, pointShadows]{CountShadows(dirLight, spotLights, pointLights)};

		m_LightShader.Clear();
		m_LightShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_LightShader["DIRLIGHT_SHADOW"] = std::to_string(dirShadow);
		m_LightShader["NUM_CASCADES"] = std::to_string(Settings::Instance().DirShadowCascadeCount());
		m_LightShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_LightShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(spotShadows);
		m_LightShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_LightShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(pointShadows);
		auto& lightShader{m_LightShader.GetPermutation()};
		auto& shadowShader{m_ShadowShader.GetPermutation()};
		auto& cubeShadowShader{m_CubeShadowShader.GetPermutation()};

		GLuint nextTexUnit{0};
		shadowShader.Use();

		nextTexUnit = RenderDirShadowMap(dirLight, camViewMat.Inverse(), camProjMat, renderables, lightShader, shadowShader, nextTexUnit);
		nextTexUnit = RenderSpotShadowMaps(spotLights, renderables, lightShader, shadowShader, spotShadows, nextTexUnit);

		cubeShadowShader.Use();
		nextTexUnit = RenderPointShadowMaps(pointLights, renderables, lightShader, cubeShadowShader, pointShadows, nextTexUnit);

		static_cast<void>(m_GBuffer.BindForReading(lightShader, nextTexUnit));

		SetAmbientData(AmbientLight::Instance(), lightShader);
		SetDirectionalData(dirLight, lightShader);
		SetSpotData(spotLights, lightShader);
		SetPointData(pointLights, lightShader);

		lightShader.SetUniform("u_CamPos", Camera::Current()->Entity()->Transform()->Position());
		lightShader.SetUniform("u_CamViewProjInv", (camViewMat * camProjMat).Inverse());

		lightShader.Use();

		glStencilFunc(GL_EQUAL, STENCIL_REF, STENCIL_AND_MASK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glDisable(GL_DEPTH_TEST);

		m_ScreenRenderBuffer.Clear();
		m_GBuffer.CopyStencilData(m_ScreenRenderBuffer.Framebuffer());
		m_ScreenRenderBuffer.BindForWriting();
		m_ScreenQuad.Draw();
	}


	auto GlDeferredRenderer::RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void
	{
		if (auto const& background{Camera::Current()->Background()}; std::holds_alternative<Skybox>(background))
		{
			glStencilFunc(GL_NOTEQUAL, STENCIL_REF, STENCIL_AND_MASK);

			m_ScreenRenderBuffer.BindForWriting();

			auto& skyboxShader{m_SkyboxShader.GetPermutation()};
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			skyboxShader.Use();

			DataManager::Instance().CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(skyboxShader);
		}
	}


	auto GlDeferredRenderer::RenderDirShadowMap(DirectionalLight const* dirLight,
	                                            Matrix4 const& camViewInvMat,
	                                            Matrix4 const& camProjMat,
	                                            std::span<RenderableData const> const renderables,
	                                            ShaderProgram& lightShader,
	                                            ShaderProgram& shadowShader,
	                                            GLuint const nextTexUnit) const -> GLuint
	{
		if (!dirLight || !dirLight->CastsShadow())
		{
			return nextTexUnit;
		}

		static std::vector<Matrix4> cascadeMats;
		cascadeMats.clear();

		auto const lightViewMat{Matrix4::LookAt(Vector3{0}, dirLight->Direction(), Vector3::Up())};
		auto const cascadeBounds{m_DirShadowMap.CalculateCascadeBounds(*Camera::Current())};
		auto const numCascades{cascadeBounds.size()};

		for (std::size_t i = 0; i < numCascades; ++i)
		{
			auto const cascadeMat{m_DirShadowMap.CascadeMatrix(cascadeBounds[i], camViewInvMat, lightViewMat, dirLight->ShadowExtension())};
			cascadeMats.push_back(cascadeMat);

			shadowShader.SetUniform("u_ViewProjMat", cascadeMat);

			m_DirShadowMap.BindForWritingAndClear(i);

			for (auto const& [renderable, instances, castsShadow] : renderables)
			{
				if (castsShadow)
				{
					renderable->SetInstanceData(instances);
					renderable->DrawWithoutMaterial(false);
				}
			}
		}

		lightShader.SetUniform("u_CascadeMatrices", cascadeMats);
		lightShader.SetUniform("u_CascadeBoundsNdc", CascadeFarBoundsNdc(camProjMat, cascadeBounds));
		return m_DirShadowMap.BindForReading(lightShader, "u_DirShadowMaps", nextTexUnit);
	}


	auto GlDeferredRenderer::RenderSpotShadowMaps(std::span<SpotLight const* const> const spotLights,
	                                              std::span<RenderableData const> const renderables,
	                                              ShaderProgram& lightShader,
	                                              ShaderProgram& shadowShader,
	                                              std::size_t const numShadows,
	                                              GLuint nextTexUnit) -> GLuint
	{
		// Not really great in terms of allocations but will do for now
		while (m_SpotShadowMaps.size() < numShadows)
		{
			m_SpotShadowMaps.push_back(std::make_unique<SpotShadowMap>());
		}

		if (numShadows * 2 < m_SpotShadowMaps.size())
		{
			m_SpotShadowMaps.resize(numShadows);
		}

		for (auto i{0ull}, shadowInd{0ull}; i < spotLights.size(); ++i)
		{
			if (spotLights[i]->CastsShadow())
			{
				auto const lightViewMat{Matrix4::LookAt(spotLights[i]->Entity()->Transform()->Position(), spotLights[i]->Entity()->Transform()->Position() + spotLights[i]->Entity()->Transform()->Forward(), Vector3::Up())};
				auto const lightProjMat{Matrix4::Perspective(math::ToRadians(spotLights[i]->OuterAngle() * 2), 1.f, 0.1f, spotLights[i]->Range())};
				auto const lightViewProjMat{lightViewMat * lightProjMat};

				shadowShader.SetUniform("u_ViewProjMat", lightViewProjMat);

				m_SpotShadowMaps[shadowInd]->BindForWritingAndClear();

				for (auto const& [renderable, instances, castsShadow] : renderables)
				{
					if (castsShadow)
					{
						renderable->SetInstanceData(instances);
						renderable->DrawWithoutMaterial(false);
					}
				}

				lightShader.SetUniform("u_SpotShadowMats[" + std::to_string(shadowInd) + "]", lightViewProjMat);
				nextTexUnit = m_SpotShadowMaps[shadowInd]->BindForReading(lightShader, "u_SpotShadowMaps[" + std::to_string(shadowInd) + "]", nextTexUnit);
				++shadowInd;
			}
		}

		return nextTexUnit;
	}


	auto GlDeferredRenderer::RenderPointShadowMaps(std::span<PointLight const* const> const pointLights,
	                                               std::span<RenderableData const> const renderables,
	                                               ShaderProgram& lightShader,
	                                               ShaderProgram& shadowShader,
	                                               std::size_t const numShadows,
	                                               GLuint nextTexUnit) -> GLuint
	{
		// Not really great in terms of allocations but will do for now
		while (m_PointShadowMaps.size() < numShadows)
		{
			m_PointShadowMaps.push_back(std::make_unique<CubeShadowMap>());
		}

		if (numShadows * 2 < m_PointShadowMaps.size())
		{
			m_PointShadowMaps.resize(numShadows);
		}

		for (auto i{0ull}, shadowInd{0ull}; i < pointLights.size(); ++i)
		{
			if (pointLights[i]->CastsShadow())
			{
				std::array<Matrix4, 6> shadowViewProjMats;

				auto const shadowProjMat{Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLights[i]->Range())};
				auto const lightTransMat{Matrix4::Translate(-pointLights[i]->Entity()->Transform()->Position())};

				static constexpr std::array cubeFaceMats
				{
					Matrix4{0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1}, // +X
					Matrix4{0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1}, // -X
					Matrix4{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // +Y
					Matrix4{1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 1}, // -Y
					Matrix4{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // +Z
					Matrix4{-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1} // -Z
				};

				std::ranges::transform(cubeFaceMats, shadowViewProjMats.begin(), [&](auto const& cubeFaceMat)
				{
					return lightTransMat * cubeFaceMat * shadowProjMat;
				});

				shadowShader.SetUniform("u_LightPos", pointLights[i]->Entity()->Transform()->Position());

				for (auto face = 0; face < 6; face++)
				{
					m_PointShadowMaps[shadowInd]->BindForWritingAndClear(face);
					shadowShader.SetUniform("u_ViewProjMat", shadowViewProjMats[face]);

					for (auto const& [renderable, instances, castsShadow] : renderables)
					{
						if (castsShadow)
						{
							renderable->SetInstanceData(instances);
							renderable->DrawWithoutMaterial(false);
						}
					}
				}

				nextTexUnit = m_PointShadowMaps[shadowInd]->BindForReading(lightShader, "u_PointShadowMaps[" + std::to_string(shadowInd) + "]", nextTexUnit);
				++shadowInd;
			}
		}

		return nextTexUnit;
	}
}
