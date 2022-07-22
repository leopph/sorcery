#include "rendering/renderers/GlDeferredRenderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Math.hpp"
#include "Matrix.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <utility>


namespace leopph::internal
{
	void GlDeferredRenderer::Render()
	{
		/*Renderer::Render();

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
			m_ShadowShader.UseCurrentPermutation();

			for (u8 i = 0; i < GetDirShadowRes().size(); i++)
			{
				GLfloat clearDepth = 1;
				glClearNamedFramebufferfv(m_DirShadowMapFramebuffers[i], GL_DEPTH, 0, &clearDepth);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DirShadowMapFramebuffers[i]);
				glViewport(0, 0, GetDirShadowRes()[i], GetDirShadowRes()[i]);

				m_ShadowShader.SetUniform("u_ViewProjMat", shadowCascades[i].WorldToClip);

				for (auto const& [renderable, instances, castsShadow] : renderNodes)
				{
					if (castsShadow)
					{
						renderable->set_instance_data(instances);
						renderable->draw_without_material(false);
					}
				}
			}
		}

		static std::vector<Matrix4> spotShadowMats;

		// DRAW SPOT SHADOW MAPS
		if (!GetCastingSpotLights().empty())
		{
			spotShadowMats.clear();

			m_ShadowShader.UseCurrentPermutation();

			auto const spotLights = GetCastingSpotLights();

			for (auto i = 0; i < spotLights.size(); i++)
			{
				auto const& pos = spotLights[i]->Owner()->get_transform().get_position();
				auto const viewMat = Matrix4::LookAt(pos, pos + spotLights[i]->Owner()->get_transform().get_forward_axis(), Vector3::Up());
				auto constexpr shadowNearClip = 0.1f;
				auto const projMat = Matrix4::Perspective(math::ToRadians(spotLights[i]->OuterAngle() * 2), 1.f, shadowNearClip, spotLights[i]->Range());
				auto const viewProjMat = viewMat * projMat;

				m_ShadowShader.SetUniform("u_ViewProjMat", viewProjMat);
				spotShadowMats.push_back(viewProjMat);

				GLfloat constexpr clearDepth = 1;
				glClearNamedFramebufferfv(m_SpotShadowMapFramebuffers[i], GL_DEPTH, 0, &clearDepth);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_SpotShadowMapFramebuffers[i]);
				glViewport(0, 0, GetSpotShadowRes(), GetSpotShadowRes());

				for (auto const& [renderable, instances, castsShadow] : renderNodes)
				{
					if (castsShadow)
					{
						renderable->set_instance_data(instances);
						renderable->draw_without_material(false);
					}
				}
			}
		}

		// DRAW POINT SHADOW MAPS
		if (!GetCastingPointLights().empty())
		{
			auto const pointLights = GetCastingPointLights();
			m_CubeShadowShader.UseCurrentPermutation();

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

				auto const translation = Matrix4::Translate(-pointLights[i]->Owner()->get_transform().get_position());
				auto const projMat = Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLights[i]->Range());

				std::array<Matrix4, 6> viewProjMats;
				std::ranges::transform(cubeFaceViewMats, viewProjMats.begin(), [&](auto const& cubeFaceMat)
				{
					return translation * cubeFaceMat * projMat;
				});

				m_CubeShadowShader.SetUniform("u_LightPos", pointLights[i]->Owner()->get_transform().get_position());

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

					m_CubeShadowShader.SetUniform("u_ViewProjMat", viewProjMats[face]);

					for (auto const& [renderable, instances, castsShadow] : renderNodes)
					{
						if (castsShadow)
						{
							renderable->set_instance_data(instances);
							renderable->draw_without_material(false);
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

		m_GeometryShader.SetUniform("u_ViewProjMat", camViewProjMat);
		m_GeometryShader.UseCurrentPermutation();

		GLuint constexpr gClearColor[]{0, 0, 0};
		GLfloat constexpr clearDepth{1};
		auto constexpr clearStencil{0};
		glClearNamedFramebufferuiv(m_Gbuffer.framebuffer, GL_COLOR, 0, gClearColor);
		glClearNamedFramebufferfi(m_Gbuffer.framebuffer, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Gbuffer.framebuffer);

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->set_instance_data(instances);
			renderable->draw_with_material(m_GeometryShader, 0, false);
		}


		// SET PIPELINE STATE FOR LIGHTING PASS
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// LIGHT PASS
		GLfloat constexpr lClearColor[]{0, 0, 0, 1};
		glClearNamedFramebufferfv(m_PingPongBuffers[0].framebuffer, GL_COLOR, 0, lClearColor);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

		auto* const ubo = glMapNamedBuffer(m_Ubos[0], GL_WRITE_ONLY);
		*static_cast<Matrix4*>(ubo) = camViewProjMat.Inverse();
		*reinterpret_cast<Vector3*>(static_cast<u8*>(ubo) + sizeof(Matrix4)) = (*GetMainCamera())->Owner()->get_transform().get_position();
		glUnmapNamedBuffer(m_Ubos[0]);

		glBindTextureUnit(0, m_Gbuffer.colorAttachment);
		glBindTextureUnit(1, m_SharedDepthStencilBuffer);
		/*
		// AMBIENT LIGHT
		m_LightShader.SetOption("AMBIENTLIGHT", true);
		m_LightShader["AMBIENTLIGHT"] = "";
		auto& ambShader = m_LightShader.GetPermutation();
		ambShader.SetUniform("u_Light", AmbientLight::Instance().Intensity());
		ambShader.Use();

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

			if (dirLight->CastsShadow())
			{
				static std::vector<std::pair<f32, f32>> cascadeBoundsNdc;
				CascadeBoundToNdc(shadowCascades, cascadeBoundsNdc);

				for (auto i = 0; i < shadowCascades.size(); i++)
				{
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].worldToClipMat", shadowCascades[i].WorldToClip);
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].nearZ", cascadeBoundsNdc[i].first);
					shader.SetUniform("u_ShadowCascades[" + std::to_string(i) + "].farZ", cascadeBoundsNdc[i].second);
					glBindTextureUnit(2 + i, m_DirShadowMapDepthAttachments[i]);
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

			for (auto i = 0; i < spotLights.size(); i++)
			{
				shader.SetUniform("u_Light.position", spotLights[i]->Owner()->Transform().Position());
				shader.SetUniform("u_Light.direction", spotLights[i]->Owner()->Transform().get_forward_axis());
				shader.SetUniform("u_Light.diffuseColor", spotLights[i]->Diffuse());
				shader.SetUniform("u_Light.specularColor", spotLights[i]->Specular());
				shader.SetUniform("u_Light.range", spotLights[i]->Range());
				shader.SetUniform("u_Light.innerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->InnerAngle())));
				shader.SetUniform("u_Light.outerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->OuterAngle())));
				shader.SetUniform("u_ShadowWorldToClip", spotShadowMats[i]);
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

			for (auto const* const spotLight : GetNonCastingSpotLights())
			{
				shader.SetUniform("u_Light.position", spotLight->Owner()->Transform().Position());
				shader.SetUniform("u_Light.direction", spotLight->Owner()->Transform().get_forward_axis());
				shader.SetUniform("u_Light.diffuseColor", spotLight->Diffuse());
				shader.SetUniform("u_Light.specularColor", spotLight->Specular());
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

			for (auto i = 0; i < pointLights.size(); i++)
			{
				shader.SetUniform("u_Light.position", pointLights[i]->Owner()->Transform().Position());
				shader.SetUniform("u_Light.diffuseColor", pointLights[i]->Diffuse());
				shader.SetUniform("u_Light.specularColor", pointLights[i]->Specular());
				shader.SetUniform("u_Light.range", pointLights[i]->Range());
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

			for (auto const* const pointLight : pointLights)
			{
				shader.SetUniform("u_Light.position", pointLight->Owner()->Transform().Position());
				shader.SetUniform("u_Light.diffuseColor", pointLight->Diffuse());
				shader.SetUniform("u_Light.specularColor", pointLight->Specular());
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
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

			m_SkyboxShader.Clear();
			auto& shader = m_SkyboxShader.GetPermutation();
			shader.Use();
			shader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(shader);
		}


		// SET TRANSPARENT PASS PIPELINE STATE
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_STENCIL_TEST);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// TRANSPARENT PASS
		GLfloat constexpr accumClear[]{0, 0, 0, 0};
		GLfloat constexpr revealClear{1};
		glClearNamedFramebufferfv(m_TransparencyBuffer.framebuffer, GL_COLOR, 0, accumClear);
		glClearNamedFramebufferfv(m_TransparencyBuffer.framebuffer, GL_COLOR, 0, &revealClear);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_TransparencyBuffer.framebuffer);

		m_ForwardObjectShader.Clear();
		if (GetDirLight())
		{
			m_ForwardObjectShader["DIRLIGHT"] = "";
		}

		m_ForwardObjectShader["NUM_SPOT"] = std::to_string(GetCastingSpotLights().size() + GetNonCastingSpotLights().size());
		m_ForwardObjectShader["NUM_POINT"] = std::to_string(GetCastingPointLights().size() + GetNonCastingPointLights().size());
		m_ForwardObjectShader["TRANSPARENT"] = "";

		auto& transpShader = m_ForwardObjectShader.GetPermutation();
		transpShader.Use();
		transpShader.SetUniform("u_ViewProjMat", camViewProjMat);
		transpShader.SetUniform("u_CamPos", (*GetMainCamera())->Owner()->Transform().Position());
		transpShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		if (GetDirLight())
		{
			auto const* const dirLight = *GetDirLight();
			transpShader.SetUniform("u_DirLight.direction", dirLight->Direction());
			transpShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
			transpShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
		}

		for (auto i = 0; auto const* const spotLight : std::ranges::join_view{std::array{GetCastingSpotLights(), GetNonCastingSpotLights()}})
		{
			auto const indStr = std::to_string(i);
			transpShader.SetUniform("u_SpotLights[" + indStr + "].position", spotLight->Owner()->Transform().Position());
			transpShader.SetUniform("u_SpotLights[" + indStr + "].direction", spotLight->Owner()->Transform().get_forward_axis());
			transpShader.SetUniform("u_SpotLights[" + indStr + "].diffuseColor", spotLight->Diffuse());
			transpShader.SetUniform("u_SpotLights[" + indStr + "].specularColor", spotLight->Specular());
			transpShader.SetUniform("u_SpotLights[" + indStr + "].range", spotLight->Range());
			transpShader.SetUniform("u_SpotLights[" + indStr + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			transpShader.SetUniform("u_SpotLights[" + indStr + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
			i++;
		}

		for (auto i = 0; auto const* const pointLight : std::ranges::join_view{std::array{GetCastingPointLights(), GetNonCastingPointLights()}})
		{
			auto const indStr = std::to_string(i);
			transpShader.SetUniform("u_PointLights[" + indStr + "].position", pointLight->Owner()->Transform().Position());
			transpShader.SetUniform("u_PointLights[" + indStr + "].diffuseColor", pointLight->Diffuse());
			transpShader.SetUniform("u_PointLights[" + indStr + "].specularColor", pointLight->Specular());
			transpShader.SetUniform("u_PointLights[" + indStr + "].range", pointLight->Range());
			i++;
		}

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->set_instance_data(instances);
			renderable->DrawWithMaterial(transpShader, 0, true);
		}


		// SET PIPELINE STATE FOR COMPOSITE PASS
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// COMPOSITE PASS
		m_TranspCompositeShader.Clear();
		auto& compShader = m_TranspCompositeShader.GetPermutation();
		compShader.Use();

		glBindTextureUnit(0, m_TransparencyBuffer.accumAttachment);
		glBindTextureUnit(1, m_TransparencyBuffer.revealAttachment);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

		DrawScreenQuad();


		// SET GAMMA PASS PIPELINE STATE
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);

		// GAMMA CORRECTION PASS
		m_GammaCorrectShader.Clear();
		auto& gammaShader = m_GammaCorrectShader.GetPermutation();
		gammaShader.SetUniform("u_GammaInverse", 1.f / GetGamma());
		gammaShader.Use();

		glBindTextureUnit(0, m_PingPongBuffers[0].colorAttachment);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[1].framebuffer);

		DrawScreenQuad();


		// COPY TO DEFAULT FRAMEBUFFER
		glBlitNamedFramebuffer(m_PingPongBuffers[1].framebuffer, 0, 0, 0, GetRenderRes().Width, GetRenderRes().Height, 0, 0, GetRenderRes().Width, GetRenderRes().Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);*/
	}



	void GlDeferredRenderer::CreateGbuffer(GLsizei const renderWidth, GLsizei const renderHeight)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Gbuffer.colorAttachment);
		glTextureStorage2D(m_Gbuffer.colorAttachment, 1, GL_RGB32UI, renderWidth, renderHeight);

		glCreateFramebuffers(1, &m_Gbuffer.framebuffer);
		glNamedFramebufferTexture(m_Gbuffer.framebuffer, GL_COLOR_ATTACHMENT0, m_Gbuffer.colorAttachment, 0);
		glNamedFramebufferTexture(m_Gbuffer.framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_SharedDepthStencilBuffer, 0);
		glNamedFramebufferDrawBuffer(m_Gbuffer.framebuffer, GL_COLOR_ATTACHMENT0);
	}



	void GlDeferredRenderer::DeleteGbuffer() const
	{
		glDeleteFramebuffers(1, &m_Gbuffer.colorAttachment);
		glDeleteTextures(1, &m_Gbuffer.colorAttachment);
	}



	void GlDeferredRenderer::CreateUbos()
	{
		glCreateBuffers(static_cast<GLsizei>(m_Ubos.size()), m_Ubos.data());

		auto constexpr uboSz0 = sizeof(Matrix4) + sizeof(Vector4);
		glNamedBufferStorage(m_Ubos[0], uboSz0, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_Ubos[0]);
	}



	void GlDeferredRenderer::DeleteUbos() const
	{
		glDeleteBuffers(static_cast<GLsizei>(m_Ubos.size()), m_Ubos.data());
	}



	void GlDeferredRenderer::OnRenderResChange(Extent2D const renderRes)
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
