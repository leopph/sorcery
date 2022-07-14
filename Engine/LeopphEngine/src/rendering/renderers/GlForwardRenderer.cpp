#include "rendering/renderers/GlForwardRenderer.hpp"

#include "AmbientLight.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

#include <ranges>
#include <string>
#include <utility>


namespace leopph::internal
{
	GlForwardRenderer::GlForwardRenderer()
	{
		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}



	auto GlForwardRenderer::Render() -> void
	{
		Renderer::Render();

		if (!GetMainCamera())
		{
			return;
		}

		static std::vector<RenderNode> renderNodes;
		renderNodes.clear();
		ExtractAndProcessInstanceData(renderNodes);


		// SET PIPELINE STATE FOR OPAQUE PASS
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glViewport(0, 0, GetRenderRes().Width, GetRenderRes().Height);


		// OPAQUE PASS

		GLfloat constexpr clearColor[]{0, 0, 0, 1};
		GLfloat constexpr clearDepth{1};
		glClearNamedFramebufferfv(m_PingPongBuffers[0].framebuffer, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(m_PingPongBuffers[0].framebuffer, GL_DEPTH, 0, &clearDepth);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

		m_ForwardObjectShader.Clear();
		if (GetDirLight())
		{
			m_ForwardObjectShader["DIRLIGHT"] = "";
		}
		m_ForwardObjectShader["NUM_SPOT"] = std::to_string(GetCastingSpotLights().size() + GetNonCastingSpotLights().size());
		m_ForwardObjectShader["NUM_POINT"] = std::to_string(GetCastingPointLights().size() + GetNonCastingPointLights().size());

		auto& opaqueShader = m_ForwardObjectShader.GetPermutation();
		opaqueShader.Use();
		opaqueShader.SetUniform("u_ViewProjMat", (*GetMainCamera())->ViewMatrix() * (*GetMainCamera())->ProjectionMatrix());
		opaqueShader.SetUniform("u_CamPos", (*GetMainCamera())->Owner()->Transform()->Position());
		opaqueShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		if (GetDirLight())
		{
			auto const* const dirLight = *GetDirLight();
			opaqueShader.SetUniform("u_DirLight.direction", dirLight->Direction());
			opaqueShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
			opaqueShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
		}

		for (auto i = 0; auto const* const spotLight : std::ranges::join_view{std::array{GetCastingSpotLights(), GetNonCastingSpotLights()}})
		{
			auto const indStr = std::to_string(i);
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].position", spotLight->Owner()->Transform()->Position());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].direction", spotLight->Owner()->Transform()->Forward());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].diffuseColor", spotLight->Diffuse());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].specularColor", spotLight->Specular());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].constant", spotLight->Constant());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].linear", spotLight->Linear());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].quadratic", spotLight->Quadratic());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].range", spotLight->Range());
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			opaqueShader.SetUniform("u_SpotLights[" + indStr + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
			i++;
		}

		for (auto i = 0; auto const* const pointLight : std::ranges::join_view{std::array{GetCastingPointLights(), GetNonCastingPointLights()}})
		{
			auto const indStr = std::to_string(i);
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].position", pointLight->Owner()->Transform()->Position());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].diffuseColor", pointLight->Diffuse());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].specularColor", pointLight->Specular());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].constant", pointLight->Constant());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].linear", pointLight->Linear());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].quadratic", pointLight->Quadratic());
			opaqueShader.SetUniform("u_PointLights[" + indStr + "].range", pointLight->Range());
			i++;
		}

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(opaqueShader, 0, true);
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
			shader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>((*GetMainCamera())->ViewMatrix())) * (*GetMainCamera())->ProjectionMatrix());
			CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(shader);
		}


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
		glBlitNamedFramebuffer(m_PingPongBuffers[1].framebuffer, 0, 0, 0, GetRenderRes().Width, GetRenderRes().Height, 0, 0, GetRenderRes().Width, GetRenderRes().Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}
