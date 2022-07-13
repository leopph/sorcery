#include "rendering/renderers/GlDeferredRenderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Math.hpp"
#include "Matrix.hpp"

#include <algorithm>
#include <array>
#include <utility>


namespace leopph::internal
{
	auto GlDeferredRenderer::Render() -> void
	{
		Renderer::Render();

		if (!GetMainCamera())
		{
			return;
		}

		static std::vector<RenderNode> renderNodes;
		renderNodes.clear();
		ExtractAndProcessInstanceData(renderNodes);


		// SET PIPELINE STATE FOR SHADOW PASSES
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);

		static std::vector<ShadowCascade> shadowCascades;

		// DRAW DIR SHADOW MAPS
		if (GetDirLight() && (*GetDirLight())->CastsShadow())
		{
			CalculateShadowCascades(shadowCascades);
			m_ShadowShader.Clear();
			auto& shadowShader = m_ShadowShader.GetPermutation();
			shadowShader.Use();

			for (u8 i = 0; i < GetDirShadowRes().size(); i++)
			{
				GLfloat clearDepth = 1;
				glClearNamedFramebufferfv(m_DirShadowMapFramebuffers[i], GL_DEPTH, 0, &clearDepth);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DirShadowMapFramebuffers[i]);
				glViewport(0, 0, GetDirShadowRes()[i], GetDirShadowRes()[i]);

				shadowShader.SetUniform("u_ViewProjMat", shadowCascades[i].WorldToClip);

				for (auto const& [renderable, instances, castsShadow] : renderNodes)
				{
					if (castsShadow)
					{
						renderable->SetInstanceData(instances);
						renderable->DrawWithoutMaterial(false);
					}
				}
			}
		}

		static std::vector<Matrix4> spotShadowMats;

		// DRAW SPOT SHADOW MAPS
		if (!GetCastingSpotLights().empty())
		{
			spotShadowMats.clear();

			m_ShadowShader.Clear();
			auto& shadowShader = m_ShadowShader.GetPermutation();
			shadowShader.Use();

			auto const spotLights = GetCastingSpotLights();

			for (auto i = 0; i < spotLights.size(); i++)
			{
				auto const& pos = spotLights[i]->Owner()->Transform()->Position();
				auto const viewMat = Matrix4::LookAt(pos, pos + spotLights[i]->Owner()->Transform()->Forward(), Vector3::Up());
				auto constexpr shadowNearClip = 0.1f;
				auto const projMat = Matrix4::Perspective(math::ToRadians(spotLights[i]->OuterAngle() * 2), 1.f, shadowNearClip, spotLights[i]->Range());
				auto const viewProjMat = viewMat * projMat;

				shadowShader.SetUniform("u_ViewProjMat", viewProjMat);
				spotShadowMats.push_back(viewProjMat);

				GLfloat constexpr clearDepth = 1;
				glClearNamedFramebufferfv(m_SpotShadowMapFramebuffers[i], GL_DEPTH, 0, &clearDepth);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_SpotShadowMapFramebuffers[i]);
				glViewport(0, 0, GetSpotShadowRes(), GetSpotShadowRes());

				for (auto const& [renderable, instances, castsShadow] : renderNodes)
				{
					if (castsShadow)
					{
						renderable->SetInstanceData(instances);
						renderable->DrawWithoutMaterial(false);
					}
				}
			}
		}

		// DRAW POINT SHADOW MAPS
		if (!GetCastingPointLights().empty())
		{
			auto const pointLights = GetCastingPointLights();

			m_CubeShadowShader.Clear();
			auto& shadowShader = m_CubeShadowShader.GetPermutation();
			shadowShader.Use();

			for (auto i = 0; i < pointLights.size(); i++)
			{
				constexpr std::array cubeFaceViewMats
				{
					Matrix4{0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1}, // +X
					Matrix4{0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1}, // -X
					Matrix4{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // +Y
					Matrix4{1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 1}, // -Y
					Matrix4{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // +Z
					Matrix4{-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1} // -Z
				};

				auto const translation = Matrix4::Translate(-pointLights[i]->Owner()->Transform()->Position());
				auto const projMat = Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLights[i]->Range());

				std::array<Matrix4, 6> viewProjMats;
				std::ranges::transform(cubeFaceViewMats, viewProjMats.begin(), [&](auto const& cubeFaceMat)
				{
					return translation * cubeFaceMat * projMat;
				});

				shadowShader.SetUniform("u_LightPos", pointLights[i]->Owner()->Transform()->Position());

				for (auto face = 0; face < 6; face++)
				{
					GLfloat constexpr clearColor[]
					{
						std::numeric_limits<GLfloat>::max(),
						std::numeric_limits<GLfloat>::max(),
						std::numeric_limits<GLfloat>::max(),
						std::numeric_limits<GLfloat>::max()
					};
					GLfloat constexpr clearDepth{1};

					glClearNamedFramebufferfv(m_PointShadowMapFramebuffers[i * 6 + face], GL_COLOR, 0, clearColor);
					glClearNamedFramebufferfv(m_PointShadowMapFramebuffers[i * 6 + face], GL_DEPTH, 0, &clearDepth);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PointShadowMapFramebuffers[i * 6 + face]);
					glViewport(0, 0, GetPointShadowRes(), GetPointShadowRes());

					shadowShader.SetUniform("u_ViewProjMat", viewProjMats[face]);

					for (auto const& [renderable, instances, castsShadow] : renderNodes)
					{
						if (castsShadow)
						{
							renderable->SetInstanceData(instances);
							renderable->DrawWithoutMaterial(false);
						}
					}
				}
			}
		}

		// SET PIPELINE STATE FOR GEOMETRY PASS
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// GEOMETRY PASS

		auto const camViewMat = (*GetMainCamera())->ViewMatrix();
		auto const camProjMat = (*GetMainCamera())->ProjectionMatrix();
		auto const camViewProjMat = camViewMat * camProjMat;

		auto& gShader = m_GeometryShader.GetPermutation();
		gShader.SetUniform("u_ViewProjMat", camViewProjMat);
		gShader.Use();

		GLuint constexpr gClearColor[]{0, 0, 0};
		GLfloat constexpr clearDepth{1};
		auto constexpr clearStencil{0};
		glClearNamedFramebufferuiv(m_GbufferFramebuffer, GL_COLOR, 0, gClearColor);
		glClearNamedFramebufferfi(m_GbufferFramebuffer, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GbufferFramebuffer);

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(gShader, 0, false);
		}


		// SET PIPELINE STATE FOR LIGHTING PASS
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// LIGHT PASS
		GLfloat constexpr lClearColor[]{0, 0, 0};
		glClearNamedFramebufferfv(m_CommonFramebuffers[1], GL_COLOR, 0, lClearColor);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_CommonFramebuffers[1]);

		auto* const ubo = glMapNamedBuffer(m_Ubos[0], GL_WRITE_ONLY);
		*static_cast<Matrix4*>(ubo) = camViewProjMat.Inverse();
		*reinterpret_cast<Vector3*>(static_cast<u8*>(ubo) + sizeof(Matrix4)) = (*GetMainCamera())->Owner()->Transform()->Position();
		glUnmapNamedBuffer(m_Ubos[0]);

		// AMBIENT LIGHT
		m_LightShader.Clear();
		m_LightShader["AMBIENTLIGHT"] = "";
		auto& ambShader = m_LightShader.GetPermutation();
		ambShader.SetUniform("u_Light", AmbientLight::Instance().Intensity());
		ambShader.SetUniform("u_NormColorGlossTex", 0);
		ambShader.SetUniform("u_DepthTex", 1);
		ambShader.Use();

		glBindTextureUnit(0, m_GbufferColorAttachments[0]);
		glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);
		DrawScreenQuad();

		// DIRECTIONAL LIGHT
		if (GetDirLight())
		{
			auto const* const dirLight = *GetDirLight();

			m_LightShader.Clear();
			m_LightShader["DIRLIGHT"] = "";

			if (dirLight->CastsShadow())
			{
				m_LightShader["SHADOW"] = "";
				m_LightShader["NUM_CASCADES"] = std::to_string(shadowCascades.size());
			}

			auto& shader = m_LightShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_Light.direction", dirLight->Direction());
			shader.SetUniform("u_Light.diffuseColor", dirLight->Diffuse());
			shader.SetUniform("u_Light.specularColor", dirLight->Specular());
			shader.SetUniform("u_NormColorGloss", 0);
			shader.SetUniform("u_DepthTex", 1);
			glBindTextureUnit(0, m_GbufferColorAttachments[0]);
			glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);

			if (dirLight->CastsShadow())
			{
				for (auto i = 0; i < shadowCascades.size(); i++)
				{
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].worldToClipMat", shadowCascades[i].WorldToClip);
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].nearZ", shadowCascades[i].Near);
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].farZ", shadowCascades[i].Far);
					shader.SetUniform("u_ShadowMaps[" + std::to_string(i) + "]", i + 1);
					glBindTextureUnit(i + 1, m_DirShadowMapDepthAttachments[i]);
				}
			}

			DrawScreenQuad();
		}

		// CASTING SPOTLIGHTS
		if (!GetCastingSpotLights().empty())
		{
			auto const spotLights = GetCastingSpotLights();

			m_LightShader.Clear();
			m_LightShader["SPOTLIGHT"] = "";
			m_LightShader["SHADOW"] = "";
			auto& shader = m_LightShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_NormColorGloss", 0);
			shader.SetUniform("u_DepthTex", 1);
			glBindTextureUnit(0, m_GbufferColorAttachments[0]);
			glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);

			for (auto i = 0; i < spotLights.size(); i++)
			{
				shader.SetUniform("u_Light.position", spotLights[i]->Owner()->Transform()->Position());
				shader.SetUniform("u_Light.direction", spotLights[i]->Owner()->Transform()->Forward());
				shader.SetUniform("u_Light.diffuseColor", spotLights[i]->Diffuse());
				shader.SetUniform("u_Light.specularColor", spotLights[i]->Specular());
				shader.SetUniform("u_Light.constant", spotLights[i]->Constant());
				shader.SetUniform("u_Light.linear", spotLights[i]->Linear());
				shader.SetUniform("u_Light.quadratic", spotLights[i]->Quadratic());
				shader.SetUniform("u_Light.range", spotLights[i]->Range());
				shader.SetUniform("u_Light.innerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->InnerAngle())));
				shader.SetUniform("u_Light.outerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->OuterAngle())));
				shader.SetUniform("u_ShadowWorldToClip", spotShadowMats[i]);
				shader.SetUniform("u_ShadowMap", 2);
				glBindTextureUnit(2, m_SpotShadowMapDepthAttachments[i]);
				DrawScreenQuad();
			}
		}

		// NON-CASTING SPOTLIGHTS
		if (!GetNonCastingSpotLights().empty())
		{
			m_LightShader.Clear();
			m_LightShader["SPOTLIGHT"] = "";
			auto& shader = m_LightShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_NormColorGloss", 0);
			shader.SetUniform("u_DepthTex", 1);
			glBindTextureUnit(0, m_GbufferColorAttachments[0]);
			glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);

			for (auto const* const spotLight : GetNonCastingSpotLights())
			{
				shader.SetUniform("u_Light.position", spotLight->Owner()->Transform()->Position());
				shader.SetUniform("u_Light.direction", spotLight->Owner()->Transform()->Forward());
				shader.SetUniform("u_Light.diffuseColor", spotLight->Diffuse());
				shader.SetUniform("u_Light.specularColor", spotLight->Specular());
				shader.SetUniform("u_Light.constant", spotLight->Constant());
				shader.SetUniform("u_Light.linear", spotLight->Linear());
				shader.SetUniform("u_Light.quadratic", spotLight->Quadratic());
				shader.SetUniform("u_Light.range", spotLight->Range());
				shader.SetUniform("u_Light.innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
				shader.SetUniform("u_Light.outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
				DrawScreenQuad();
			}
		}

		// CASTING POINTLIGHTS
		if (!GetCastingPointLights().empty())
		{
			auto const pointLights = GetCastingPointLights();

			m_LightShader.Clear();
			m_LightShader["POINTLIGHT"] = "";
			m_LightShader["SHADOW"] = "";
			auto& shader = m_LightShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_NormColorGloss", 0);
			shader.SetUniform("u_DepthTex", 1);
			glBindTextureUnit(0, m_GbufferColorAttachments[0]);
			glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);

			for (auto i = 0; i < pointLights.size(); i++)
			{
				shader.SetUniform("u_Light.position", pointLights[i]->Owner()->Transform()->Position());
				shader.SetUniform("u_Light.diffuseColor", pointLights[i]->Diffuse());
				shader.SetUniform("u_Light.specularColor", pointLights[i]->Specular());
				shader.SetUniform("u_Light.constant", pointLights[i]->Constant());
				shader.SetUniform("u_Light.linear", pointLights[i]->Linear());
				shader.SetUniform("u_Light.quadratic", pointLights[i]->Quadratic());
				shader.SetUniform("u_Light.range", pointLights[i]->Range());
				shader.SetUniform("u_ShadowMap", 2);
				glBindTextureUnit(2, m_PointShadowMapColorAttachments[i]);
				DrawScreenQuad();
			}
		}

		// NON-CASTING POINTLIGHTS
		if (!GetNonCastingPointLights().empty())
		{
			auto const pointLights = GetNonCastingPointLights();

			m_LightShader.Clear();
			m_LightShader["POINTLIGHT"] = "";
			auto& shader = m_LightShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_NormColorGloss", 0);
			shader.SetUniform("u_DepthTex", 1);
			glBindTextureUnit(0, m_GbufferColorAttachments[0]);
			glBindTextureUnit(1, m_CommonDepthStencilAttachments[0]);

			for (auto const* const pointLight : pointLights)
			{
				shader.SetUniform("u_Light.position", pointLight->Owner()->Transform()->Position());
				shader.SetUniform("u_Light.diffuseColor", pointLight->Diffuse());
				shader.SetUniform("u_Light.specularColor", pointLight->Specular());
				shader.SetUniform("u_Light.constant", pointLight->Constant());
				shader.SetUniform("u_Light.linear", pointLight->Linear());
				shader.SetUniform("u_Light.quadratic", pointLight->Quadratic());
				shader.SetUniform("u_Light.range", pointLight->Range());
				DrawScreenQuad();
			}
		}

		// SKYBOX PASS
		if (auto const& background = (*GetMainCamera())->Background(); std::holds_alternative<Skybox>(background))
		{
			// SET PIPELINE STATE
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_EQUAL, 0, 1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_CommonFramebuffers[1]);

			m_SkyboxShader.Clear();
			auto& shader = m_SkyboxShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(shader);
		}


		// TRANSPARENT PASS
		// TODO
	}



	/*auto GlDeferredRenderer::RenderTransparent(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderNode> const& renderNodes, DirectionalLight const* dirLight, std::vector<SpotLight const*> const& spotLights, std::vector<PointLight const*> const& pointLights) -> void
	{
		m_ForwardObjectShader.Clear();
		m_ForwardObjectShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_ForwardObjectShader["DIRLIGHT_SHADOW"] = std::to_string(false);
		m_ForwardObjectShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_ForwardObjectShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(0);
		m_ForwardObjectShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_ForwardObjectShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(0);
		m_ForwardObjectShader["TRANSPARENT"] = std::to_string(true);

		auto& forwardShader{m_ForwardObjectShader.GetPermutation()};

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

		auto& compositeShader{m_TranspCompositeShader.GetPermutation()};
		compositeShader.Use();

		m_TransparencyBuffer.BindForReading(compositeShader, 0);
		m_RenderBuffer.BindForWriting();

		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

		glDisable(GL_DEPTH_TEST);

		DrawScreenQuad();
	}*/


	auto GlDeferredRenderer::CreateGbuffer(GLsizei const renderWidth, GLsizei const renderHeight) -> void
	{
		m_GbufferColorAttachments.resize(1);
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_GbufferColorAttachments.size()), m_GbufferColorAttachments.data());

		glTextureStorage2D(m_GbufferColorAttachments[0], 1, GL_RGB32UI, renderWidth, renderHeight);

		glCreateFramebuffers(1, &m_GbufferFramebuffer);
		glNamedFramebufferDrawBuffer(m_GbufferFramebuffer, GL_COLOR_ATTACHMENT0);

		glNamedFramebufferTexture(m_GbufferFramebuffer, GL_COLOR_ATTACHMENT0, m_GbufferColorAttachments[0], 0);
		glNamedFramebufferTexture(m_GbufferFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_CommonDepthStencilAttachments[0], 0);
	}


	auto GlDeferredRenderer::DeleteGbuffer() const -> void
	{
		glDeleteFramebuffers(1, &m_GbufferFramebuffer);
		glDeleteTextures(static_cast<GLsizei>(m_GbufferColorAttachments.size()), m_GbufferColorAttachments.data());
	}


	auto GlDeferredRenderer::CreateUbos() -> void
	{
		glCreateBuffers(static_cast<GLsizei>(m_Ubos.size()), m_Ubos.data());

		auto constexpr uboSz0 = sizeof(Matrix4) + sizeof(Vector4);
		glNamedBufferStorage(m_Ubos[0], uboSz0, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_Ubos[0]);
	}


	auto GlDeferredRenderer::DeleteUbos() const -> void
	{
		glDeleteBuffers(static_cast<GLsizei>(m_Ubos.size()), m_Ubos.data());
	}


	auto GlDeferredRenderer::OnRenderResChange(Extent2D const renderRes) -> void
	{
		GlRenderer::OnRenderResChange(renderRes);
		DeleteGbuffer();
		auto const& [renderWidth, renderHeight] = GetRenderRes();
		CreateGbuffer(static_cast<GLsizei>(renderWidth), static_cast<GLsizei>(renderHeight));
	}



	GlDeferredRenderer::GlDeferredRenderer()
	{
		auto const [renderWidth, renderHeight] = [this]() -> std::pair<GLsizei, GLsizei>
		{
			auto const& [width, height] = GetRenderRes();
			return {static_cast<GLsizei>(width), static_cast<GLsizei>(height)};
		}();

		CreateGbuffer(renderWidth, renderHeight);
		CreateUbos();
	}


	GlDeferredRenderer::~GlDeferredRenderer()
	{
		DeleteUbos();
		DeleteGbuffer();
	}
}
