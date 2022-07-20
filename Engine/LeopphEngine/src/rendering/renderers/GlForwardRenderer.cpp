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
			auto ret = sizeof(UboGenericData);
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

		if (m_PerFrameUbos[m_PerFrameUboInd].size < uboDataSize)
		{
			DeleteUbo(m_PerFrameUboInd);
			CreateUbo(m_PerFrameUboInd, uboDataSize);
		}

		// WRITE DATA TO UBO
		auto* const uboData = m_PerFrameUbos[m_PerFrameUboInd].mapping;

		UboGenericData genericData;
		genericData.viewProjMat = camViewProjMat;
		genericData.ambientLight = GetAmbLight();
		genericData.cameraPosition = (*GetMainCamera())->Owner()->Transform()->Position();
		*reinterpret_cast<UboGenericData*>(uboData) = genericData;
		auto offset = sizeof genericData;

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

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_PerFrameUbos[m_PerFrameUboInd].name, 0, uboDataSize);
		m_PerFrameUboInd = (m_PerFrameUboInd + 1) % m_PerFrameUbos.size();


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

		if (GetDirLight())
		{
			m_ForwardObjectShader.Option("DIRLIGHT", true);
		}
		m_ForwardObjectShader.Option("NUM_SPOT", GetCastingSpotLights().size() + GetNonCastingSpotLights().size());
		m_ForwardObjectShader.Option("NUM_POINT", GetCastingPointLights().size() + GetNonCastingPointLights().size());
		m_ForwardObjectShader.Option("TRANSPARENT", false);
		m_ForwardObjectShader.UseCurrentPermutation();

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(m_ForwardObjectShader, 0, false);
		}


		// SKYBOX PASS
		if (auto const& background = (*GetMainCamera())->Background(); std::holds_alternative<Skybox>(background))
		{
			// SET PIPELINE STATE
			glDepthMask(GL_FALSE);

			m_SkyboxShader.UseCurrentPermutation();

			m_SkyboxShader.Uniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);

			CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(m_SkyboxShader);
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

		m_ForwardObjectShader.Option("TRANSPARENT", true);
		m_ForwardObjectShader.UseCurrentPermutation();

		for (auto const& [renderable, instances, castsShadow] : renderNodes)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(m_ForwardObjectShader, 0, true);
		}


		// SET COMPOSITE PASS PIPELINE STATE
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);


		// COMPOSITE PASS
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[0].framebuffer);

		glBindTextureUnit(0, m_TransparencyBuffer.accumAttachment);
		glBindTextureUnit(1, m_TransparencyBuffer.revealAttachment);

		m_TranspCompositeShader.UseCurrentPermutation();
		DrawScreenQuad();


		// SET GAMMA PASS PIPELINE STATE
		glDisable(GL_BLEND);


		// GAMMA CORRECTION PASS
		m_GammaCorrectShader.UseCurrentPermutation();

		m_GammaCorrectShader.Uniform("u_GammaInverse", 1.f / GetGamma());

		glBindTextureUnit(0, m_PingPongBuffers[0].colorAttachment);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[1].framebuffer);

		DrawScreenQuad();


		// COPY TO DEFAULT FRAMEBUFFER
		glBlitNamedFramebuffer(m_PingPongBuffers[1].framebuffer, 0, 0, 0, GetRenderRes().Width, GetRenderRes().Height, 0, 0, GetRenderRes().Width, GetRenderRes().Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto GlForwardRenderer::CreateUbo(u64 const index, u64 const size) -> void
	{
		glCreateBuffers(1, &m_PerFrameUbos[index].name);
		glNamedBufferStorage(m_PerFrameUbos[index].name, size, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		m_PerFrameUbos[index].mapping = static_cast<u8*>(glMapNamedBufferRange(m_PerFrameUbos[index].name, 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
		m_PerFrameUbos[index].size = size;

		#ifndef NDEBUG
		if (!m_PerFrameUbos[index].mapping)
		{
			auto const errMsg = "Failed to map UBO[" + std::to_string(index) + "].";
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif
	}


	auto GlForwardRenderer::DeleteUbo(u64 const index) const -> void
	{
		glUnmapNamedBuffer(m_PerFrameUbos[index].name);
		glDeleteBuffers(1, &m_PerFrameUbos[index].name);
	}



	GlForwardRenderer::GlForwardRenderer()
	{
		static_assert(sizeof(UboDirData) == 48);
		static_assert(sizeof(UboSpotData) == 80);
		static_assert(sizeof(UboPointData) == 48);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}



	GlForwardRenderer::~GlForwardRenderer()
	{
		for (auto i = 0; i < m_PerFrameUbos.size(); i++)
		{
			DeleteUbo(i);
		}
	}
}
