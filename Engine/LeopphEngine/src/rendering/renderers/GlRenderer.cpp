#include "rendering/renderers/GlRenderer.hpp"

#include "InternalContext.hpp"
#include "Logger.hpp"
#include "ScreenQuad.hpp"
#include "SettingsImpl.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/renderers/GlDeferredRenderer.hpp"
#include "rendering/renderers/GlForwardRenderer.hpp"
#include "windowing/WindowImpl.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	auto GlRenderer::Create() -> std::unique_ptr<GlRenderer>
	{
		opengl::Init();

		switch (GetSettingsImpl()->GetGraphicsPipeline())
		{
			case Settings::GraphicsPipeline::Forward:
				return std::make_unique<GlForwardRenderer>();
			case Settings::GraphicsPipeline::Deferred:
				return std::make_unique<GlDeferredRenderer>();
		}

		auto const errMsg = "The selected rendering pipeline is not supported for the current graphics API.";
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}



	auto GlRenderer::CreateRenderObject(MeshGroup const& meshGroup) -> RenderObject*
	{
		auto glMeshGroup = std::make_unique<GlMeshGroup>(meshGroup);
		auto* const ret = glMeshGroup.get();
		m_RenderObjects.push_back(std::move(glMeshGroup));
		return ret;
	}



	auto GlRenderer::DeleteRenderObject(RenderObject* renderObject) -> void
	{
		std::erase_if(m_RenderObjects, [renderObject](auto const& elem)
		{
			return elem.get() == renderObject;
		});
	}



	auto GlRenderer::CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> GlSkyboxImpl*
	{
		for (auto const& skyboxImpl : m_SkyboxImpls)
		{
			if (skyboxImpl->AllPaths() == allPaths)
			{
				return skyboxImpl.get();
			}
		}

		m_SkyboxImpls.emplace_back(std::make_unique<GlSkyboxImpl>(std::move(allPaths)));
		return m_SkyboxImpls.back().get();
	}



	auto GlRenderer::DestroySkyboxImpl(GlSkyboxImpl const* skyboxImpl) -> void
	{
		std::erase_if(m_SkyboxImpls, [skyboxImpl](auto const& elem)
		{
			return elem.get() == skyboxImpl;
		});
	}



	auto GlRenderer::ExtractAndProcessInstanceData(std::vector<RenderNode>& out) -> void
	{
		m_NonInstancedMatrixCache.clear();
		m_InstancedMatrixCache.clear();

		for (auto const& glMeshGroup : m_RenderObjects)
		{
			glMeshGroup->SortMeshes();
			auto instancedShadow = false;

			for (auto const renderNodes = glMeshGroup->ExtractRenderInstanceData(); auto const& [worldTransform, normalTransform, isInstanced, castsShadow] : renderNodes)
			{
				// Transpose matrices because they go into an OpenGL buffer.
				auto const worldTrafoTransp = worldTransform.Transposed();
				auto const normTrafoTransp = normalTransform.Transposed();

				if (isInstanced)
				{
					// Accumulate instanced matrices
					m_InstancedMatrixCache[glMeshGroup.get()].emplace_back(worldTrafoTransp, normTrafoTransp);
					instancedShadow = instancedShadow || castsShadow;
				}
				else
				{
					// output a node for a non-instanced render
					m_NonInstancedMatrixCache.push_back(std::make_pair(worldTrafoTransp, normTrafoTransp));
					RenderNode renderInstanceData;
					renderInstanceData.RenderObject = glMeshGroup.get();
					renderInstanceData.Instances = std::span{std::begin(m_NonInstancedMatrixCache) + m_NonInstancedMatrixCache.size() - 1, 1};
					renderInstanceData.CastsShadow = castsShadow;
					out.push_back(renderInstanceData);
				}
			}

			// output a single node for an instanced render
			RenderNode renderInstanceData;
			renderInstanceData.RenderObject = glMeshGroup.get();
			renderInstanceData.Instances = m_InstancedMatrixCache[glMeshGroup.get()];
			renderInstanceData.CastsShadow = instancedShadow;
			out.push_back(renderInstanceData);
		}
	}



	auto GlRenderer::CascadeBoundToNdc(std::span<ShadowCascade const> const cascades, std::vector<std::pair<f32, f32>>& out) const -> void
	{
		out.resize(cascades.size());
		auto const camProjMat = (*GetMainCamera())->ProjectionMatrix();

		for (auto i = 0; i < cascades.size(); i++)
		{
			// This relies heavily on the projection matrix being row-major and projecting from a LH base to NDC.

			auto const near = (cascades[i].Near * camProjMat[2][2] + camProjMat[3][2]) / cascades[i].Near;
			auto const far = (cascades[i].Far * camProjMat[2][2] + camProjMat[3][2]) / cascades[i].Far;
			out[i] = {near, far};
		}
	}



	auto GlRenderer::CreateScreenQuad() -> void
	{
		using VertPosElemtType = decltype(g_ScreenQuadVertices)::value_type;

		glCreateBuffers(1, &m_ScreenQuadVbo);
		glNamedBufferStorage(m_ScreenQuadVbo, sizeof(VertPosElemtType) * g_ScreenQuadVertices.size(), g_ScreenQuadVertices.data(), 0);

		glCreateVertexArrays(1, &m_ScreenQuadVao);
		glVertexArrayVertexBuffer(m_ScreenQuadVao, 0, m_ScreenQuadVbo, 0, 2 * sizeof(VertPosElemtType));

		glVertexArrayAttribBinding(m_ScreenQuadVao, 0, 0);
		glVertexArrayAttribFormat(m_ScreenQuadVao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(m_ScreenQuadVao, 0);
	}



	auto GlRenderer::DrawScreenQuad() const -> void
	{
		glBindVertexArray(m_ScreenQuadVao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}



	auto GlRenderer::DeleteScreenQuad() const -> void
	{
		glDeleteVertexArrays(1, &m_ScreenQuadVao);
		glDeleteBuffers(1, &m_ScreenQuadVbo);
	}



	auto GlRenderer::CreateRenderTargets(GLsizei const renderWidth, GLsizei const renderHeight) -> void
	{
		// Shared depth-stencil buffer
		glCreateTextures(GL_TEXTURE_2D, 1, &m_SharedDepthStencilBuffer);
		glTextureStorage2D(m_SharedDepthStencilBuffer, 1, GL_DEPTH24_STENCIL8, renderWidth, renderHeight);

		// Ping-pong buffers
		for (auto& [framebuffer, colorAttachment] : m_PingPongBuffers)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment);
			glTextureStorage2D(colorAttachment, 1, GL_RGB8, renderWidth, renderHeight);

			glCreateFramebuffers(1, &framebuffer);
			glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, colorAttachment, 0);
			glNamedFramebufferTexture(framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_SharedDepthStencilBuffer, 0);
			glNamedFramebufferDrawBuffer(framebuffer, GL_COLOR_ATTACHMENT0);
		}

		// Transparency buffer
		glCreateTextures(GL_TEXTURE_2D, 1, &m_TransparencyBuffer.accumAttachment);
		glTextureStorage2D(m_TransparencyBuffer.accumAttachment, 1, GL_RGBA16F, renderWidth, renderHeight);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TransparencyBuffer.revealAttachment);
		glTextureStorage2D(m_TransparencyBuffer.revealAttachment, 1, GL_R8, renderWidth, renderHeight);

		glCreateFramebuffers(1, &m_TransparencyBuffer.framebuffer);
		glNamedFramebufferTexture(m_TransparencyBuffer.framebuffer, GL_COLOR_ATTACHMENT0, m_TransparencyBuffer.accumAttachment, 0);
		glNamedFramebufferTexture(m_TransparencyBuffer.framebuffer, GL_COLOR_ATTACHMENT1, m_TransparencyBuffer.revealAttachment, 0);
		glNamedFramebufferTexture(m_TransparencyBuffer.framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_SharedDepthStencilBuffer, 0);
		glNamedFramebufferDrawBuffers(m_TransparencyBuffer.framebuffer, 2, std::array<GLenum, 2>{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}.data());
	}



	auto GlRenderer::DeleteRenderTargets() const -> void
	{
		// Transparency buffer
		glDeleteFramebuffers(1, &m_TransparencyBuffer.framebuffer);
		glDeleteTextures(1, &m_TransparencyBuffer.revealAttachment);
		glDeleteTextures(1, &m_TransparencyBuffer.accumAttachment);

		// Ping-pong buffers
		for (auto const& [framebuffer, colorAttachment] : m_PingPongBuffers)
		{
			glDeleteFramebuffers(1, &framebuffer);
			glDeleteTextures(1, &colorAttachment);
		}

		// Shared depth-stencil buffer
		glDeleteTextures(1, &m_SharedDepthStencilBuffer);
	}



	auto GlRenderer::CreateDirShadowMaps(std::span<u16 const> const resolutions) -> void
	{
		m_DirShadowMapFramebuffers.resize(resolutions.size());
		glCreateFramebuffers(static_cast<GLsizei>(m_DirShadowMapFramebuffers.size()), m_DirShadowMapFramebuffers.data());

		m_DirShadowMapDepthAttachments.resize(resolutions.size());
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_DirShadowMapDepthAttachments.size()), m_DirShadowMapDepthAttachments.data());

		for (u64 i = 0; i < resolutions.size(); i++)
		{
			glTextureStorage2D(m_DirShadowMapDepthAttachments[i], 1, GL_DEPTH_COMPONENT32F, resolutions[i], resolutions[i]);

			glTextureParameteri(m_DirShadowMapDepthAttachments[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(m_DirShadowMapDepthAttachments[i], GL_TEXTURE_COMPARE_FUNC, GL_LESS);

			glNamedFramebufferTexture(m_DirShadowMapFramebuffers[i], GL_DEPTH_ATTACHMENT, m_DirShadowMapDepthAttachments[i], 0);
		}
	}



	auto GlRenderer::DeleteDirShadowMaps() const -> void
	{
		glDeleteFramebuffers(static_cast<GLsizei>(m_DirShadowMapFramebuffers.size()), m_DirShadowMapFramebuffers.data());
		glDeleteTextures(static_cast<GLsizei>(m_DirShadowMapDepthAttachments.size()), m_DirShadowMapDepthAttachments.data());
	}


	auto GlRenderer::CreateAppendSpotShadowMaps(u16 const resolution, u8 const count) -> void
	{
		auto const oldNumMaps = static_cast<u8>(m_SpotShadowMapFramebuffers.size());

		m_SpotShadowMapFramebuffers.resize(oldNumMaps + count);
		glCreateFramebuffers(static_cast<GLsizei>(count), m_SpotShadowMapFramebuffers.data() + oldNumMaps);

		m_SpotShadowMapDepthAttachments.resize(oldNumMaps + count);
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(count), m_SpotShadowMapDepthAttachments.data() + oldNumMaps);

		for (u64 i = oldNumMaps; i < m_SpotShadowMapDepthAttachments.size(); i++)
		{
			glTextureStorage2D(m_SpotShadowMapDepthAttachments[i], 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution));
			glTextureParameteri(m_SpotShadowMapDepthAttachments[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(m_SpotShadowMapDepthAttachments[i], GL_TEXTURE_COMPARE_FUNC, GL_LESS);

			glNamedFramebufferTexture(m_SpotShadowMapFramebuffers[i], GL_DEPTH_ATTACHMENT, m_SpotShadowMapDepthAttachments[i], 0);
		}
	}



	auto GlRenderer::DeleteSpotShadowMaps() const -> void
	{
		glDeleteFramebuffers(static_cast<GLsizei>(m_SpotShadowMapFramebuffers.size()), m_SpotShadowMapFramebuffers.data());
		glDeleteTextures(static_cast<GLsizei>(m_SpotShadowMapDepthAttachments.size()), m_SpotShadowMapDepthAttachments.data());
	}


	auto GlRenderer::CreateAppendPointShadowMaps(u16 const resolution, u8 const count) -> void
	{
		auto const oldNumMaps = static_cast<u8>(m_PointShadowMapFramebuffers.size());

		m_PointShadowMapFramebuffers.resize(oldNumMaps + count * 6);
		glCreateFramebuffers(static_cast<GLsizei>(count * 6), m_PointShadowMapFramebuffers.data() + oldNumMaps);

		m_PointShadowMapColorAttachments.resize(oldNumMaps + count);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, static_cast<GLsizei>(count), m_PointShadowMapColorAttachments.data() + oldNumMaps);

		m_PointShadowMapDepthAttachments.resize(oldNumMaps + count);
		glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(count), m_PointShadowMapDepthAttachments.data() + oldNumMaps);

		for (u64 i = oldNumMaps; i < m_PointShadowMapColorAttachments.size(); i++)
		{
			glTextureStorage2D(m_PointShadowMapColorAttachments[i], 1, GL_R32F, static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution));
			glTextureStorage2D(m_PointShadowMapDepthAttachments[i], 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(resolution), static_cast<GLsizei>(resolution));

			for (auto j = 0; j < 6; j++)
			{
				auto const bufInd = i * 6 + j;
				glNamedFramebufferTextureLayer(m_PointShadowMapFramebuffers[bufInd], GL_COLOR_ATTACHMENT0, m_PointShadowMapColorAttachments[i], 0, j);
				glNamedFramebufferTexture(m_PointShadowMapFramebuffers[bufInd], GL_DEPTH_ATTACHMENT, m_PointShadowMapDepthAttachments[i], 0);

				glNamedFramebufferDrawBuffer(m_PointShadowMapFramebuffers[bufInd], GL_COLOR_ATTACHMENT0);
			}
		}
	}



	auto GlRenderer::DeletePointShadowMaps() const -> void
	{
		glDeleteFramebuffers(static_cast<GLsizei>(m_PointShadowMapFramebuffers.size()), m_PointShadowMapFramebuffers.data());
		glDeleteTextures(static_cast<GLsizei>(m_PointShadowMapColorAttachments.size()), m_PointShadowMapColorAttachments.data());
		glDeleteTextures(static_cast<GLsizei>(m_PointShadowMapDepthAttachments.size()), m_PointShadowMapDepthAttachments.data());
	}



	auto GlRenderer::OnRenderResChange(Extent2D const renderRes) -> void
	{
		DeleteRenderTargets();
		CreateRenderTargets(static_cast<GLsizei>(renderRes.Width), static_cast<GLsizei>(renderRes.Height));
	}



	auto GlRenderer::OnDirShadowResChange(std::span<u16 const> const resolutions) -> void
	{
		DeleteDirShadowMaps();
		CreateDirShadowMaps(resolutions);
	}



	auto GlRenderer::OnSpotShadowResChange(u16 const resolution) -> void
	{
		auto const numMaps = static_cast<u8>(m_SpotShadowMapFramebuffers.size());
		DeleteSpotShadowMaps();
		m_SpotShadowMapFramebuffers.clear();
		m_SpotShadowMapDepthAttachments.clear();
		CreateAppendSpotShadowMaps(resolution, numMaps);
	}



	auto GlRenderer::OnPointShadowResChange(u16 const resolution) -> void
	{
		auto const numMaps = static_cast<u8>(m_PointShadowMapFramebuffers.size());
		DeletePointShadowMaps();
		m_PointShadowMapFramebuffers.clear();
		m_PointShadowMapColorAttachments.clear();
		m_PointShadowMapDepthAttachments.clear();
		CreateAppendPointShadowMaps(resolution, numMaps);
	}



	auto GlRenderer::OnDetermineShadowMapCountRequirements(u8 const spot, u8 const point) -> void
	{
		if (m_SpotShadowMapFramebuffers.size() < spot)
		{
			auto const deficit = static_cast<u8>(spot - m_SpotShadowMapFramebuffers.size());
			CreateAppendSpotShadowMaps(GetSpotShadowRes(), deficit);
		}

		if (m_PointShadowMapFramebuffers.size() < point)
		{
			auto const deficit = static_cast<u8>(point - m_PointShadowMapFramebuffers.size());
			CreateAppendPointShadowMaps(GetPointShadowRes(), deficit);
		}
	}



	GlRenderer::GlRenderer()
	{
		ShaderFamily::AddGlobalOption("DIRLIGHT_NO_SHADOW", 0, 1);
		ShaderFamily::AddGlobalOption("NUM_DIRLIGHT_SHADOW_CASCADE", 0, 3);
		ShaderFamily::AddGlobalOption("NUM_SPOT_NO_SHADOW", 0, 8);
		ShaderFamily::AddGlobalOption("NUM_SPOT_SHADOW", 0, 8);
		ShaderFamily::AddGlobalOption("NUM_POINT_NO_SHADOW", 0, 8);
		ShaderFamily::AddGlobalOption("NUM_POINT_SHADOW", 0, 8);

		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		CreateScreenQuad();

		auto const& [renderWidth, renderHeight] = GetRenderRes();
		CreateRenderTargets(static_cast<GLsizei>(renderWidth), static_cast<GLsizei>(renderHeight));
		CreateDirShadowMaps(GetDirShadowRes());
	}



	GlRenderer::~GlRenderer() noexcept
	{
		DeleteScreenQuad();
		DeleteRenderTargets();
	}
}
