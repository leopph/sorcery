#include "rendering/renderers/GlForwardRenderer.hpp"

#include "Camera.hpp"
#include "Entity.hpp"
#include "Logger.hpp"
#include "Util.hpp"

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
		auto const camViewMatInv = camViewMat.Inverse();
		auto const camProjMat = (*GetMainCamera())->ProjectionMatrix();
		auto const camProjMatInv = camProjMat.Inverse();
		auto const camViewProjMat = camViewMat * camProjMat;
		auto const camViewProjMatInv = camViewProjMat.Inverse();


		// Resize lighting UBO if the light configuration changed since the previous frame
		auto const lightingUboDataSize = [this]
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

		if (m_LightingUbos[m_PerFrameUboInd].size < lightingUboDataSize)
		{
			DeleteUbo(m_LightingUbos[m_PerFrameUboInd]);
			CreateUbo(m_LightingUbos[m_PerFrameUboInd], lightingUboDataSize);
		}


		// Write transformations to transformation UBO
		auto* const transformUbo = reinterpret_cast<Matrix4*>(m_TransformUbos[m_PerFrameUboInd].mapping);
		transformUbo[0] = camViewMat;
		transformUbo[1] = camViewMatInv;
		transformUbo[2] = camProjMat;
		transformUbo[3] = camProjMatInv;
		transformUbo[4] = camViewProjMat;
		transformUbo[5] = camViewProjMatInv;


		// WRITE DATA TO LIGHTING UBO
		auto* const lightingUbo = m_LightingUbos[m_PerFrameUboInd].mapping;

		UboGenericData genericData;
		genericData.cameraPosition = (*GetMainCamera())->Owner()->Transform()->Position();
		genericData.ambientLight = GetAmbLight();
		*reinterpret_cast<UboGenericData*>(lightingUbo) = genericData;
		auto offset = sizeof genericData;

		if (GetDirLight())
		{
			auto const* const dirLight = *GetDirLight();
			UboDirData dirData;
			dirData.direction = dirLight->Direction();
			dirData.diffuse = dirLight->Diffuse();
			dirData.specular = dirLight->Specular();
			*reinterpret_cast<UboDirData*>(lightingUbo + offset) = dirData;
			offset += sizeof dirData;
		}

		for (auto const& spotLights : {GetNonCastingSpotLights(), GetCastingSpotLights()})
		{
			for (auto const* const spotLight : spotLights)
			{
				UboSpotData spotData;
				spotData.position = spotLight->Owner()->Transform()->Position();
				spotData.direction = spotLight->Owner()->Transform()->Forward();
				spotData.diffuse = spotLight->Diffuse();
				spotData.specular = spotLight->Specular();
				spotData.range = spotLight->Range();
				spotData.innerCos = math::Cos(math::ToRadians(spotLight->InnerAngle()));
				spotData.outerCos = math::Cos(math::ToRadians(spotLight->OuterAngle()));
				*reinterpret_cast<UboSpotData*>(lightingUbo + offset) = spotData;
				offset += sizeof spotData;
			}
		}

		for (auto const& pointLights : {GetNonCastingPointLights(), GetCastingPointLights()})
		{
			for (auto const* const pointLight : pointLights)
			{
				UboPointData pointData;
				pointData.position = pointLight->Owner()->Transform()->Position();
				pointData.diffuse = pointLight->Diffuse();
				pointData.specular = pointLight->Specular();
				pointData.range = pointLight->Range();
				*reinterpret_cast<UboPointData*>(lightingUbo + offset) = pointData;
				offset += sizeof pointData;
			}
		}

		// BIND UBOS
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_TransformUbos[m_PerFrameUboInd].name, 0, TRANSFORM_UBO_SIZE);
		glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_LightingUbos[m_PerFrameUboInd].name, 0, ClampCast<GLsizei>(lightingUboDataSize));
		m_PerFrameUboInd = (m_PerFrameUboInd + 1) % m_LightingUbos.size();


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

		if (auto const dirLight = GetDirLight())
		{
			if ((*dirLight)->CastsShadow())
			{
				ShaderFamily::SetGlobalOption("DIRLIGHT_NO_SHADOW", false);
				ShaderFamily::SetGlobalOption("NUM_DIRLIGHT_SHADOW_CASCADE", 3);
			}
			else
			{
				ShaderFamily::SetGlobalOption("DIRLIGHT_NO_SHADOW", true);
				ShaderFamily::SetGlobalOption("NUM_DIRLIGHT_SHADOW_CASCADE", 0);
			}
		}

		ShaderFamily::SetGlobalOption("NUM_SPOT_NO_SHADOW", GetNonCastingSpotLights().size());
		ShaderFamily::SetGlobalOption("NUM_SPOT_SHADOW", GetCastingSpotLights().size());
		ShaderFamily::SetGlobalOption("NUM_POINT_NO_SHADOW", GetNonCastingPointLights().size());
		ShaderFamily::SetGlobalOption("NUM_POINT_SHADOW", GetCastingPointLights().size());
		ShaderFamily::SetGlobalOption("TRANSPARENT", false);
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

			m_SkyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);

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

		ShaderFamily::SetGlobalOption("TRANSPARENT", true);
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

		m_GammaCorrectShader.SetUniform("u_GammaInverse", 1.f / GetGamma());

		glBindTextureUnit(0, m_PingPongBuffers[0].colorAttachment);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PingPongBuffers[1].framebuffer);

		DrawScreenQuad();


		// COPY TO DEFAULT FRAMEBUFFER
		glBlitNamedFramebuffer(m_PingPongBuffers[1].framebuffer, 0, 0, 0, GetRenderRes().Width, GetRenderRes().Height, 0, 0, GetRenderRes().Width, GetRenderRes().Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto GlForwardRenderer::CreateUbo(MappedBuffer& ubo, u64 const size) -> void
	{
		glCreateBuffers(1, &ubo.name);
		glNamedBufferStorage(ubo.name, size, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		ubo.mapping = static_cast<u8*>(glMapNamedBufferRange(ubo.name, 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
		ubo.size = size;

		#ifndef NDEBUG
		if (!ubo.mapping)
		{
			auto const errMsg = "Failed to map UBO.";
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif
	}


	auto GlForwardRenderer::DeleteUbo(MappedBuffer const& ubo) -> void
	{
		glUnmapNamedBuffer(ubo.name);
		glDeleteBuffers(1, &ubo.name);
	}



	GlForwardRenderer::GlForwardRenderer()
	{
		static_assert(sizeof(UboDirData) == 48);
		static_assert(sizeof(UboSpotData) == 80);
		static_assert(sizeof(UboPointData) == 48);

		// Create transformation ubos

		for (auto& ubo : m_TransformUbos)
		{
			CreateUbo(ubo, TRANSFORM_UBO_SIZE);
		}

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}



	GlForwardRenderer::~GlForwardRenderer()
	{
		for (auto const& ubos : {std::cref(m_TransformUbos), std::cref(m_LightingUbos)})
		{
			for (auto const& ubo : ubos.get())
			{
				DeleteUbo(ubo);
			}
		}
	}
}
