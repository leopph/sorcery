#include "GlRenderer.hpp"

#include "GlDeferredRenderer.hpp"
#include "GlForwardRenderer.hpp"
#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/Logger.hpp"
#include "../opengl/OpenGl.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	auto GlRenderer::Create() -> std::unique_ptr<GlRenderer>
	{
		switch (Settings::Instance().GetGraphicsPipeline())
		{
			case Settings::GraphicsPipeline::Forward:
				return std::make_unique<GlForwardRenderer>();
			case Settings::GraphicsPipeline::Deferred:
				return std::make_unique<GlDeferredRenderer>();
		}

		auto const errMsg{"The selected rendering pipeline is not supported for the current graphics API."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}


	GlRenderer::GlRenderer()
	{
		opengl::Init();
	}


	auto GlRenderer::CollectRenderables(std::vector<RenderableData>& renderables) -> void
	{
		for (auto const& [meshId, groupAndInstances] : DataManager::Instance().MeshGroupsAndActiveInstances())
		{
			static std::vector<std::pair<Matrix4, Matrix4>> instanceMatrices;

			GlMeshGroup* glMeshGroup{nullptr};

			if (auto const test = groupAndInstances.RenderObject.lock()) // if this is a dangling pointer we just skip it
			{
				glMeshGroup = test.get();
			}
			else
			{
				continue;
			}

			instanceMatrices.clear();
			glMeshGroup->SortMeshes();
			auto castsShadow = false;

			for (auto const& instance : groupAndInstances.ActiveRenderComponents)
			{
				auto const& [modelMat, normalMat]{instance->Owner()->Transform()->Matrices()};

				if (instance->Instanced())
				{
					instanceMatrices.emplace_back(modelMat.Transposed(), normalMat.Transposed());
					castsShadow = castsShadow || instance->CastsShadow();
				}
				else
				{
					renderables.emplace_back(glMeshGroup, std::vector{std::make_pair(modelMat.Transposed(), normalMat.Transposed())}, instance->CastsShadow());
				}
			}

			if (!instanceMatrices.empty())
			{
				renderables.emplace_back(glMeshGroup, instanceMatrices, castsShadow);
			}
		}
	}


	auto GlRenderer::CollectSpotLights(std::vector<SpotLight const*>& spotLights) -> void
	{
		spotLights = DataManager::Instance().ActiveSpotLights();
		std::ranges::sort(spotLights, CompareLightsByDistToCam);
		if (auto const limit{Settings::Instance().MaxSpotLightCount()};
			spotLights.size() > limit)
		{
			spotLights.resize(limit);
		}
	}


	auto GlRenderer::CollectPointLights(std::vector<PointLight const*>& pointLights) -> void
	{
		pointLights = DataManager::Instance().ActivePointLights();
		std::ranges::sort(pointLights, CompareLightsByDistToCam);
		if (auto const limit{Settings::Instance().MaxPointLightCount()};
			pointLights.size() > limit)
		{
			pointLights.resize(limit);
		}
	}


	auto GlRenderer::CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<CascadedShadowMap::CascadeBounds const> const cascadeBounds) -> std::span<float const>
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


	auto GlRenderer::CompareLightsByDistToCam(Light const* left, Light const* right) -> bool
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
