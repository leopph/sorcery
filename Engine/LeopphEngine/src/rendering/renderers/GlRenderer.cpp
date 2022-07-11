#include "rendering/renderers/GlRenderer.hpp"

#include "Camera.hpp"
#include "DataManager.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"
#include "SettingsImpl.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/renderers/GlDeferredRenderer.hpp"
#include "rendering/renderers/GlForwardRenderer.hpp"
#include "windowing/WindowImpl.hpp"

#include <algorithm>
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


	auto GlRenderer::ExtractSpotLightsCurrentCamera(std::vector<SpotLight const*>& out) -> void
	{
		auto const spotLights = GetDataManager()->ActiveSpotLights();
		out.assign(std::begin(spotLights), std::end(spotLights));
		std::ranges::sort(out, CompareLightsByDistToCurrentCam);

		if (auto const szLimit{GetSettingsImpl()->MaxSpotLightCount()}; out.size() > szLimit)
		{
			out.resize(szLimit);
		}
	}


	auto GlRenderer::ExtractPointLightsCurrentCamera(std::vector<PointLight const*>& out) -> void
	{
		auto const pointLights = GetDataManager()->ActivePointLights();
		out.assign(std::begin(pointLights), std::end(pointLights));
		std::ranges::sort(out, CompareLightsByDistToCurrentCam);

		if (auto const limit{GetSettingsImpl()->MaxPointLightCount()}; out.size() > limit)
		{
			out.resize(limit);
		}
	}


	auto GlRenderer::CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<GlCascadedShadowMap::CascadeBounds const> const cascadeBounds) -> std::span<float const>
	{
		static std::vector<float> farBounds;
		farBounds.clear();

		// Essentially we calculate (0, 0, bounds.Far, 1) * camProjMat, do a perspective divide, and take the Z component.
		std::ranges::transform(cascadeBounds, std::back_inserter(farBounds), [&](auto const& bounds)
		{
			return (bounds.Far * camProjMat[2][2] + camProjMat[3][2]) / bounds.Far; // This relies heavily on the projection matrix being row-major and projecting from a LH base to NDC.
		});

		return farBounds;
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


	auto GlRenderer::ApplyGammaCorrection() -> void
	{
		m_GammaCorrectedBuffer.Clear();
		m_GammaCorrectedBuffer.BindForWriting();
		glBindTextureUnit(0, m_RenderBuffer.ColorBuffer());

		auto& shader = m_GammaCorrectShader.GetPermutation();
		shader.SetUniform("u_GammaInverse", 1.f / GetSettingsImpl()->Gamma());
		shader.SetUniform("u_Image", 0);
		shader.Use();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		
		m_ScreenQuad.Draw();
	}


	auto GlRenderer::Present() const -> void
	{
		auto const* const window = GetWindowImpl();
		glBlitNamedFramebuffer(m_GammaCorrectedBuffer.Framebuffer(), 0, 0, 0, m_GammaCorrectedBuffer.Width(), m_GammaCorrectedBuffer.Height(), 0, 0, static_cast<GLint>(window->Width()), static_cast<GLint>(window->Height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto GlRenderer::CompareLightsByDistToCurrentCam(Light const* left, Light const* right) -> bool
	{
		auto const& camPosition{Camera::Current()->Owner()->Transform()->Position()};
		auto const& leftDistance{Vector3::Distance(camPosition, left->Owner()->Transform()->Position())};
		auto const& rightDistance{Vector3::Distance(camPosition, right->Owner()->Transform()->Position())};
		if (std::abs(leftDistance - rightDistance) < std::numeric_limits<float>::epsilon())
		{
			return leftDistance < rightDistance;
		}
		return left < right;
	}
}
