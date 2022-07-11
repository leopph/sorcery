#include "rendering/renderers/GlDeferredRenderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "DataManager.hpp"
#include "InternalContext.hpp"
#include "Math.hpp"
#include "Matrix.hpp"
#include "SettingsImpl.hpp"

#include <algorithm>
#include <array>


namespace leopph::internal
{
	GlDeferredRenderer::GlDeferredRenderer() :
		m_GeometryShader{
			{
				{ShaderFamily::GeometryPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::GeometryPassFragSrc, ShaderType::Fragment}
			}
		},
		m_LightShader{
			{
				{ShaderFamily::Pos2DPassthroughVertSrc, ShaderType::Vertex},
				{ShaderFamily::LightPassFragSrc, ShaderType::Fragment}
			}
		},
		m_ForwardShader{
			{
				{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
			}
		},
		m_CompositeShader{
			{
				{ShaderFamily::Pos2DPassthroughVertSrc, ShaderType::Vertex},
				{ShaderFamily::TranspCompositeFragSrc, ShaderType::Fragment}
			}
		}
	{
		glDepthFunc(GL_LEQUAL);
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

		static std::vector<RenderNode> renderNodes;
		static std::vector<SpotLight const*> spotLights;
		static std::vector<PointLight const*> pointLights;

		renderNodes.clear();
		ExtractAndProcessInstanceData(renderNodes);
		spotLights.clear();
		ExtractSpotLightsCurrentCamera(spotLights);
		pointLights.clear();
		ExtractPointLightsCurrentCamera(pointLights);

		auto const currCamViewMat = Camera::Current()->ViewMatrix();
		auto const currCamProjMat = Camera::Current()->ProjectionMatrix();

		RenderGeometry(currCamViewMat, currCamProjMat, renderNodes);
		RenderLights(currCamViewMat, currCamProjMat, renderNodes, spotLights, pointLights);
		RenderSkybox(currCamViewMat, currCamProjMat);
		RenderTransparent(currCamViewMat, currCamProjMat, renderNodes, GetDataManager()->DirectionalLight(), spotLights, pointLights);
		ApplyGammaCorrection();
		Present();
	}


	auto GlDeferredRenderer::RenderGeometry(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderNode> const& renderables) -> void
	{
		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, STENCIL_DRAW_TAG, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		m_GBuffer.Clear();
		m_GBuffer.BindForWriting();

		auto& shader{m_GeometryShader.GetPermutation()};
		shader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		shader.Use();

		for (auto const& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(shader, 0, false);
		}
	}


	auto GlDeferredRenderer::RenderLights(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::span<RenderNode const> const renderNodes, std::span<SpotLight const*> const spotLights, std::span<PointLight const*> const pointLights) -> void
	{
		auto const* const dirLight = GetDataManager()->DirectionalLight();
		auto const [dirShadow, spotShadows, pointShadows] = CountShadows(dirLight, spotLights, pointLights);

		m_LightShader.Clear();
		m_LightShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_LightShader["DIRLIGHT_SHADOW"] = std::to_string(dirShadow);
		m_LightShader["NUM_CASCADES"] = std::to_string(GetSettingsImpl()->DirShadowCascadeCount());
		m_LightShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_LightShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(spotShadows);
		m_LightShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_LightShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(pointShadows);
		auto& lightShader{m_LightShader.GetPermutation()};
		auto& shadowShader{m_ShadowShader.GetPermutation()};
		auto& cubeShadowShader{m_CubeShadowShader.GetPermutation()};

		GLuint nextTexUnit{0};
		shadowShader.Use();

		nextTexUnit = RenderDirShadowMap(dirLight, camViewMat.Inverse(), camProjMat, renderNodes, lightShader, shadowShader, nextTexUnit);
		nextTexUnit = RenderSpotShadowMaps(spotLights, renderNodes, lightShader, shadowShader, spotShadows, nextTexUnit);

		cubeShadowShader.Use();
		nextTexUnit = RenderPointShadowMaps(pointLights, renderNodes, lightShader, cubeShadowShader, pointShadows, nextTexUnit);

		static_cast<void>(m_GBuffer.BindForReading(lightShader, nextTexUnit));

		SetAmbientData(AmbientLight::Instance(), lightShader);
		SetDirectionalData(dirLight, lightShader);
		SetSpotData(spotLights, lightShader);
		SetPointData(pointLights, lightShader);

		lightShader.SetUniform("u_CamPos", Camera::Current()->Owner()->Transform()->Position());
		lightShader.SetUniform("u_CamViewProjInv", (camViewMat * camProjMat).Inverse());

		lightShader.Use();

		m_RenderBuffer.Clear();
		m_RenderBuffer.BindForWriting();

		glDisable(GL_BLEND);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, STENCIL_DRAW_TAG, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glBlitNamedFramebuffer(m_GBuffer.Framebuffer(), m_RenderBuffer.Framebuffer(), 0, 0, m_GBuffer.Width(), m_GBuffer.Height(), 0, 0, m_RenderBuffer.Width(), m_RenderBuffer.Height(), GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		DrawScreenQuad();
	}


	auto GlDeferredRenderer::RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void
	{
		if (auto const& background{Camera::Current()->Background()}; std::holds_alternative<Skybox>(background))
		{
			glDisable(GL_BLEND);

			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			glDisable(GL_STENCIL_TEST);

			m_RenderBuffer.BindForWriting();

			auto& skyboxShader{m_SkyboxShader.GetPermutation()};
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			skyboxShader.Use();

			static_cast<GlRenderer*>(GetRenderer())->CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(skyboxShader);
		}
	}


	auto GlDeferredRenderer::RenderDirShadowMap(DirectionalLight const* dirLight,
	                                            Matrix4 const& camViewInvMat,
	                                            Matrix4 const& camProjMat,
	                                            std::span<RenderNode const> const renderNodes,
	                                            ShaderProgram& lightShader,
	                                            ShaderProgram& shadowShader,
	                                            GLuint const nextTexUnit) const -> GLuint
	{
		if (!dirLight || !dirLight->CastsShadow())
		{
			return nextTexUnit;
		}

		auto const camera = Camera::Current();
		auto const lightViewMat = Matrix4::LookAt(Vector3{0}, dirLight->Direction(), Vector3::Up());
		auto const cascadeBounds = GlCascadedShadowMap::CalculateCascadeBounds(camera->NearClipPlane(), camera->FarClipPlane());
		auto const numCascades = cascadeBounds.size();
		auto const cascadeMats = GlCascadedShadowMap::CascadeMatrix(camera->Frustum(), cascadeBounds, lightViewMat, camViewInvMat * lightViewMat, dirLight->ShadowExtension());

		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glDisable(GL_STENCIL_TEST);

		for (std::size_t i = 0; i < numCascades; ++i)
		{
			shadowShader.SetUniform("u_ViewProjMat", cascadeMats[i]);

			m_DirShadowMap.BindForWriting(i);
			m_DirShadowMap.Clear();

			for (auto const& [renderable, instances, castsShadow] : renderNodes)
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
	                                              std::span<RenderNode const> const renderNodes,
	                                              ShaderProgram& lightShader,
	                                              ShaderProgram& shadowShader,
	                                              std::size_t const numShadows,
	                                              GLuint nextTexUnit) -> GLuint
	{
		// Not really great in terms of allocations but will do for now
		while (m_SpotShadowMaps.size() < numShadows)
		{
			m_SpotShadowMaps.push_back(std::make_unique<GlSpotShadowMap>());
		}

		if (numShadows * 2 < m_SpotShadowMaps.size())
		{
			m_SpotShadowMaps.resize(numShadows);
		}

		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glDisable(GL_STENCIL_TEST);

		for (auto i{0ull}, shadowInd{0ull}; i < spotLights.size(); ++i)
		{
			if (spotLights[i]->CastsShadow())
			{
				auto const lightViewMat{Matrix4::LookAt(spotLights[i]->Owner()->Transform()->Position(), spotLights[i]->Owner()->Transform()->Position() + spotLights[i]->Owner()->Transform()->Forward(), Vector3::Up())};
				auto const lightProjMat{Matrix4::Perspective(math::ToRadians(spotLights[i]->OuterAngle() * 2), 1.f, 0.1f, spotLights[i]->Range())};
				auto const lightViewProjMat{lightViewMat * lightProjMat};

				shadowShader.SetUniform("u_ViewProjMat", lightViewProjMat);

				m_SpotShadowMaps[shadowInd]->BindForWritingAndClear();

				for (auto const& [renderable, instances, castsShadow] : renderNodes)
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
	                                               std::span<RenderNode const> const renderNodes,
	                                               ShaderProgram& lightShader,
	                                               ShaderProgram& shadowShader,
	                                               std::size_t const numShadows,
	                                               GLuint nextTexUnit) -> GLuint
	{
		// Not really great in terms of allocations but will do for now
		while (m_PointShadowMaps.size() < numShadows)
		{
			m_PointShadowMaps.push_back(std::make_unique<GlCubeShadowMap>());
		}

		if (numShadows * 2 < m_PointShadowMaps.size())
		{
			m_PointShadowMaps.resize(numShadows);
		}

		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glDisable(GL_STENCIL_TEST);

		for (auto i{0ull}, shadowInd{0ull}; i < pointLights.size(); ++i)
		{
			if (pointLights[i]->CastsShadow())
			{
				std::array<Matrix4, 6> shadowViewProjMats;

				auto const shadowProjMat{Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLights[i]->Range())};
				auto const lightTransMat{Matrix4::Translate(-pointLights[i]->Owner()->Transform()->Position())};

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

				shadowShader.SetUniform("u_LightPos", pointLights[i]->Owner()->Transform()->Position());

				for (auto face = 0; face < 6; face++)
				{
					m_PointShadowMaps[shadowInd]->BindForWritingAndClear(face);
					shadowShader.SetUniform("u_ViewProjMat", shadowViewProjMats[face]);

					for (auto const& [renderable, instances, castsShadow] : renderNodes)
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


	auto GlDeferredRenderer::RenderTransparent(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderNode> const& renderNodes, DirectionalLight const* dirLight, std::vector<SpotLight const*> const& spotLights, std::vector<PointLight const*> const& pointLights) -> void
	{
		m_ForwardShader.Clear();
		m_ForwardShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_ForwardShader["DIRLIGHT_SHADOW"] = std::to_string(false);
		m_ForwardShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_ForwardShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(0);
		m_ForwardShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_ForwardShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(0);
		m_ForwardShader["TRANSPARENT"] = std::to_string(true);

		auto& forwardShader{m_ForwardShader.GetPermutation()};

		forwardShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		forwardShader.SetUniform("u_CamPos", Camera::Current()->Owner()->Transform()->Position());

		SetAmbientData(AmbientLight::Instance(), forwardShader);
		SetDirectionalData(dirLight, forwardShader);
		SetSpotDataIgnoreShadow(spotLights, forwardShader);
		SetPointDataIgnoreShadow(pointLights, forwardShader);

		forwardShader.Use();

		m_TransparencyBuffer.Clear();
		m_TransparencyBuffer.BindForWriting();

		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glDisable(GL_STENCIL_TEST);

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(forwardShader, 0, true);
		}

		auto& compositeShader{m_CompositeShader.GetPermutation()};
		compositeShader.Use();

		m_TransparencyBuffer.BindForReading(compositeShader, 0);
		m_RenderBuffer.BindForWriting();

		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

		glDisable(GL_DEPTH_TEST);

		DrawScreenQuad();
	}
}
