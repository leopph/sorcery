#include "rendering/renderers/GlRenderer.hpp"

#include "DataManager.hpp"
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
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	auto GlRenderer::Create() -> std::unique_ptr<GlRenderer>
	{
		opengl::Init();

		switch (GetSettingsImpl()->GetGraphicsPipeline())
		{
			/*case Settings::GraphicsPipeline::Forward:
				return std::make_unique<GlForwardRenderer>();*/
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



	auto GlRenderer::SetAmbientData(AmbientLight const& light, ShaderProgram& lightShader) -> void
	{
		lightShader.SetUniform("u_AmbientLight", light.Intensity());
	}



	auto GlRenderer::SetDirectionalData(DirectionalLight const* dirLight, ShaderProgram& shader) -> void
	{
		if (!dirLight)
		{
			return;
		}

		shader.SetUniform("u_DirLight.direction", dirLight->Direction());
		shader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
		shader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
	}



	auto GlRenderer::SetSpotData(std::span<SpotLight const* const> const spotLights, ShaderProgram& shader) -> void
	{
		constexpr auto shadowArrayName{"u_SpotLightsShadow["};
		constexpr auto noShadowArrayName{"u_SpotLightsNoShadow["};

		auto noShadowInd{0ull};
		auto shadowInd{0ull};

		for (auto const spotLight : spotLights)
		{
			auto const arrayPrefix{
				[&]() -> std::string
				{
					if (spotLight->CastsShadow())
					{
						auto ret{shadowArrayName + std::to_string(shadowInd) + "]."};
						++shadowInd;
						return ret;
					}
					auto ret{noShadowArrayName + std::to_string(noShadowInd) + "]."};
					++noShadowInd;
					return ret;
				}()
			};

			shader.SetUniform(arrayPrefix + "position", spotLight->Owner()->Transform()->Position());
			shader.SetUniform(arrayPrefix + "direction", spotLight->Owner()->Transform()->Forward());
			shader.SetUniform(arrayPrefix + "diffuseColor", spotLight->Diffuse());
			shader.SetUniform(arrayPrefix + "specularColor", spotLight->Specular());
			shader.SetUniform(arrayPrefix + "constant", spotLight->Constant());
			shader.SetUniform(arrayPrefix + "linear", spotLight->Linear());
			shader.SetUniform(arrayPrefix + "quadratic", spotLight->Quadratic());
			shader.SetUniform(arrayPrefix + "range", spotLight->Range());
			shader.SetUniform(arrayPrefix + "innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			shader.SetUniform(arrayPrefix + "outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}
	}



	auto GlRenderer::SetSpotDataIgnoreShadow(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void
	{
		for (auto i = 0; i < spotLights.size(); i++)
		{
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].position", spotLights[i]->Owner()->Transform()->Position());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].direction", spotLights[i]->Owner()->Transform()->Forward());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].diffuseColor", spotLights[i]->Diffuse());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].specularColor", spotLights[i]->Specular());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].constant", spotLights[i]->Constant());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].linear", spotLights[i]->Linear());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].quadratic", spotLights[i]->Quadratic());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "].range", spotLights[i]->Range());
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "]innerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->InnerAngle())));
			shader.SetUniform("u_SpotLightsNoShadow[" + std::to_string(i) + "]outerAngleCosine", math::Cos(math::ToRadians(spotLights[i]->OuterAngle())));
		}
	}



	auto GlRenderer::SetPointData(std::span<PointLight const* const> const pointLights, ShaderProgram& shader) -> void
	{
		constexpr auto shadowArrayName{"u_PointLightsShadow["};
		constexpr auto noShadowArrayName{"u_PointLightsNoShadow["};

		auto noShadowInd{0ull};
		auto shadowInd{0ull};

		for (auto const pointLight : pointLights)
		{
			auto const arrayPrefix{
				[&]() -> std::string
				{
					if (pointLight->CastsShadow())
					{
						auto ret{shadowArrayName + std::to_string(shadowInd) + "]."};
						++shadowInd;
						return ret;
					}
					auto ret{noShadowArrayName + std::to_string(noShadowInd) + "]."};
					++noShadowInd;
					return ret;
				}()
			};

			shader.SetUniform(arrayPrefix + "position", pointLight->Owner()->Transform()->Position());
			shader.SetUniform(arrayPrefix + "diffuseColor", pointLight->Diffuse());
			shader.SetUniform(arrayPrefix + "specularColor", pointLight->Specular());
			shader.SetUniform(arrayPrefix + "constant", pointLight->Constant());
			shader.SetUniform(arrayPrefix + "linear", pointLight->Linear());
			shader.SetUniform(arrayPrefix + "quadratic", pointLight->Quadratic());
			shader.SetUniform(arrayPrefix + "range", pointLight->Range());
		}
	}



	auto GlRenderer::SetPointDataIgnoreShadow(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void
	{
		for (auto i = 0; i < pointLights.size(); i++)
		{
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].position", pointLights[i]->Owner()->Transform()->Position());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].diffuseColor", pointLights[i]->Diffuse());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].specularColor", pointLights[i]->Specular());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].constant", pointLights[i]->Constant());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].linear", pointLights[i]->Linear());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].quadratic", pointLights[i]->Quadratic());
			shader.SetUniform("u_PointLightsNoShadow[" + std::to_string(i) + "].range", pointLights[i]->Range());
		}
	}



	auto GlRenderer::CountShadows(DirectionalLight const* const dirLight, std::span<SpotLight const* const> const spotLights, std::span<PointLight const* const> const pointLights) -> ShadowCount
	{
		auto const dirShadow{dirLight != nullptr && dirLight->CastsShadow()};
		auto const spotShadows{
			std::accumulate(spotLights.begin(), spotLights.end(), 0ull, [](auto const sum, auto const elem)
			{
				if (elem->CastsShadow())
				{
					return sum + 1;
				}
				return sum;
			})
		};
		auto const pointShadows{
			std::accumulate(pointLights.begin(), pointLights.end(), 0ull, [](auto const sum, auto const elem)
			{
				if (elem->CastsShadow())
				{
					return sum + 1;
				}
				return sum;
			})
		};
		return {dirShadow, spotShadows, pointShadows};
	}


	auto GlRenderer::DrawDirShadowMaps() -> void
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
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
		// 2 for transparency, 2 for post process ping-pong
		auto constexpr numColorAttachments = 4;

		m_CommonColorAttachments.resize(numColorAttachments);
		glCreateTextures(GL_TEXTURE_2D, numColorAttachments, m_CommonColorAttachments.data());

		auto constexpr numRgbAttachments = 2;
		static_assert(numRgbAttachments <= numColorAttachments);

		for (auto i = 0; i < numRgbAttachments; i++)
		{
			glTextureStorage2D(m_CommonColorAttachments[i], 1, GL_RGB8, renderWidth, renderHeight);
		}

		// Transparency accumulator
		glTextureStorage2D(m_CommonColorAttachments[numRgbAttachments], 1, GL_RGBA16F, renderWidth, renderHeight);
		// Transparency revealage
		glTextureStorage2D(m_CommonColorAttachments[numRgbAttachments + 1], 1, GL_R8, renderWidth, renderHeight);

		// Potentially shared between framebuffers
		auto constexpr numDepthStencilAttachments = 1;

		m_CommonDepthStencilAttachments.resize(numDepthStencilAttachments);
		glCreateTextures(GL_TEXTURE_2D, numDepthStencilAttachments, m_CommonDepthStencilAttachments.data());

		for (auto i = 0; i < numDepthStencilAttachments; i++)
		{
			glTextureStorage2D(m_CommonDepthStencilAttachments[i], 1, GL_DEPTH24_STENCIL8, renderWidth, renderHeight);
		}

		// 1 for transparency MRT, 2 for post process ping-pong
		auto constexpr numFramebuffers = 3;

		m_CommonFramebuffers.resize(numFramebuffers);
		glCreateFramebuffers(numFramebuffers, m_CommonFramebuffers.data());

		// Transparency MRT
		std::array<GLenum, 2> const transpDrawBufs{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glNamedFramebufferDrawBuffers(m_CommonFramebuffers[0], transpDrawBufs.size(), transpDrawBufs.data());
		glNamedFramebufferTexture(m_CommonFramebuffers[0], GL_COLOR_ATTACHMENT0, m_CommonColorAttachments[2], 0);
		glNamedFramebufferTexture(m_CommonFramebuffers[0], GL_COLOR_ATTACHMENT1, m_CommonColorAttachments[3], 0);
		glNamedFramebufferTexture(m_CommonFramebuffers[0], GL_DEPTH_STENCIL_ATTACHMENT, m_CommonDepthStencilAttachments[0], 0);

		// Ping-pong buffers
		for (auto frameBuf = 1, tex = 0; frameBuf < numFramebuffers && tex < 2; frameBuf++, tex++)
		{
			glNamedFramebufferDrawBuffer(m_CommonFramebuffers[frameBuf], GL_COLOR_ATTACHMENT0);
			glNamedFramebufferTexture(m_CommonFramebuffers[frameBuf], GL_COLOR_ATTACHMENT0, m_CommonColorAttachments[tex], 0);
		}
	}



	auto GlRenderer::DeleteRenderTargets() const -> void
	{
		glDeleteFramebuffers(static_cast<GLsizei>(m_CommonFramebuffers.size()), m_CommonFramebuffers.data());
		glDeleteTextures(static_cast<GLsizei>(m_CommonColorAttachments.size()), m_CommonColorAttachments.data());
		glDeleteTextures(static_cast<GLsizei>(m_CommonDepthStencilAttachments.size()), m_CommonDepthStencilAttachments.data());
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
