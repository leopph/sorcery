#include "rendering/renderers/GlForwardRenderer.hpp"

#include "Camera.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

#include <ranges>
#include <string>
#include <utility>


namespace leopph::internal
{
	auto GlForwardRenderer::Render() -> void
	{
		GlRenderer::Render();

		if (!GetMainCamera())
		{
			return;
		}

		static std::vector<RenderNode> renderNodes;
		renderNodes.clear();
		ExtractAndProcessInstanceData(renderNodes);

		auto const camViewMat = (*GetMainCamera())->ViewMatrix();
		auto const camProjMat = (*GetMainCamera())->ProjectionMatrix();
		auto const camViewProjMat = camViewMat * camProjMat;


		// RESIZE PER FRAME UBO IF NEEDED
		auto const uboDataSize = [this]
		{
			u64 ret = sizeof(UboGenericData);
			if (GetDirLight())
			{
				ret += sizeof(UboDirData);
			}
			ret += GetCastingSpotLights().size() * sizeof(UboSpotData);
			ret += GetNonCastingSpotLights().size() * sizeof(UboSpotData);
			ret += GetCastingPointLights().size() * sizeof(UboPointData);
			ret += GetNonCastingPointLights().size() * sizeof(UboPointData);
			return ret;
		}();

		if (m_PerFrameUbo.size < uboDataSize)
		{
			glNamedBufferData(m_PerFrameUbo.name, uboDataSize, nullptr, GL_DYNAMIC_DRAW);
			m_PerFrameUbo.size = uboDataSize;
		}

		// WRITE DATA TO UBO
		auto* const uboData = static_cast<u8*>(glMapNamedBufferRange(m_PerFrameUbo.name, 0, uboDataSize, GL_MAP_WRITE_BIT));

		UboGenericData genericData;
		genericData.viewProjMat = camViewProjMat;
		genericData.ambientLight = GetAmbLight();
		genericData.cameraPosition = (*GetMainCamera())->Owner()->Transform()->Position();
		*reinterpret_cast<UboGenericData*>(uboData) = genericData;
		u64 offset = sizeof genericData;

		if (GetDirLight())
		{
			auto const* const dirLight = *GetDirLight();
			UboDirData dirData;
			dirData.direction = dirLight->Direction();
			dirData.diffuse = dirLight->Diffuse();
			dirData.specular = dirLight->Specular();
			*reinterpret_cast<UboDirData*>(uboData + offset) = dirData;
			offset += sizeof dirData;
		}

		for (auto const* const spotLight : std::ranges::join_view{std::array{GetCastingSpotLights(), GetNonCastingSpotLights()}})
		{
			UboSpotData spotData;
			spotData.position = spotLight->Owner()->Transform()->Position();
			spotData.direction = spotLight->Owner()->Transform()->Forward();
			spotData.diffuse = spotLight->Diffuse();
			spotData.specular = spotLight->Specular();
			spotData.range = spotLight->Range();
			spotData.innerCos = math::Cos(math::ToRadians(spotLight->InnerAngle()));
			spotData.outerCos = math::Cos(math::ToRadians(spotLight->OuterAngle()));
			*reinterpret_cast<UboSpotData*>(uboData + offset) = spotData;
			offset += sizeof spotData;
		}

		for (auto const* const pointLight : std::ranges::join_view{std::array{GetCastingPointLights(), GetNonCastingPointLights()}})
		{
			UboPointData pointData;
			pointData.position = pointLight->Owner()->Transform()->Position();
			pointData.diffuse = pointLight->Diffuse();
			pointData.specular = pointLight->Specular();
			pointData.range = pointLight->Range();
			*reinterpret_cast<UboPointData*>(uboData + offset) = pointData;
			offset += sizeof pointData;
		}

		glUnmapNamedBuffer(m_PerFrameUbo.name);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_PerFrameUbo.name, 0, uboDataSize);


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

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(opaqueShader, 0, false);
		}


		// SKYBOX PASS
		if (auto const& background = (*GetMainCamera())->Background(); std::holds_alternative<Skybox>(background))
		{
			// SET PIPELINE STATE
			glDepthMask(GL_FALSE);

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
		glDepthMask(GL_FALSE);


		// TRANSPARENT PASS

		GLfloat constexpr clearAccum[]{0, 0, 0, 0};
		GLfloat constexpr clearReveal{1};
		glClearNamedFramebufferfv(m_TransparencyBuffer.framebuffer, GL_COLOR, 0, clearAccum);
		glClearNamedFramebufferfv(m_TransparencyBuffer.framebuffer, GL_COLOR, 1, &clearReveal);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_TransparencyBuffer.framebuffer);

		m_ForwardObjectShader["TRANSPARENT"] = "";
		auto& transpShader = m_ForwardObjectShader.GetPermutation();
		transpShader.Use();

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(transpShader, 0, true);
		}


		// SET COMPOSITE PASS PIPELINE STATE
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);


		// COMPOSITE PASS
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

		glBindTextureUnit(0, m_TransparencyBuffer.accumAttachment);
		glBindTextureUnit(1, m_TransparencyBuffer.revealAttachment);

		m_TranspCompositeShader.Clear();
		auto const& compShader = m_TranspCompositeShader.GetPermutation();
		compShader.Use();

		DrawScreenQuad();


		// SET GAMMA PASS PIPELINE STATE
		glDisable(GL_BLEND);


		// GAMMA CORRECTION PASS
		m_GammaCorrectShader.Clear();
		auto& gammaShader = m_GammaCorrectShader.GetPermutation();
		gammaShader.Use();

		gammaShader.SetUniform("u_GammaInverse", 1.f / GetGamma());

		glBindTextureUnit(0, m_PingPongBuffers[0].colorAttachment);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[1].framebuffer);

		DrawScreenQuad();


		// COPY TO DEFAULT FRAMEBUFFER
		glBlitNamedFramebuffer(m_PingPongBuffers[1].framebuffer, 0, 0, 0, GetRenderRes().Width, GetRenderRes().Height, 0, 0, GetRenderRes().Width, GetRenderRes().Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}



	GlForwardRenderer::GlForwardRenderer()
	{
		static_assert(sizeof(UboDirData) == 48);
		static_assert(sizeof(UboSpotData) == 80);
		static_assert(sizeof(UboPointData) == 48);
		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
		glCreateBuffers(1, &m_PerFrameUbo.name);
	}



	GlForwardRenderer::~GlForwardRenderer()
	{
		glDeleteBuffers(1, &m_PerFrameUbo.name);
	}
}
