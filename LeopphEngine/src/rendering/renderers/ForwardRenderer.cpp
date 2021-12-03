#include "ForwardRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/Light.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"
#include "../../util/logger.h"

#include <glad/gl.h>

#include <cstddef>
#include <string>
#include <utility>



namespace leopph::impl
{
	ForwardRenderer::ForwardRenderer() :
		m_ObjectShader
	{
		{
			{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
			{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
		}
	},
		m_ObjectShaderInstanced
	{
		{
			{ShaderFamily::ObjectVertInstancedSrc, ShaderType::Vertex},
			{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
		}
	},
		m_SkyboxShader
	{
		{
			{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
			{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
		}
	},
		m_ShadowShader
	{
		{
			{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
		}
	},
		m_ShadowShaderInstanced
	{
		{
			{ShaderFamily::ShadowMapVertInstancedSrc, ShaderType::Vertex}
		}
	}
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}


	void ForwardRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active == nullptr)
		{
			return;
		}

		UpdateMatrices();

		const auto camViewMat{Camera::Active->ViewMatrix()};
		const auto camProjMat{Camera::Active->ProjectionMatrix()};

		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		RenderShadedObjects(camViewMat, camProjMat, pointLights, spotLights);
		RenderSkybox(camViewMat, camProjMat);
	}


	void ForwardRenderer::RenderShadedObjects(const Matrix4& camViewMat, const Matrix4& camProjMat,
											  const std::vector<const PointLight*>& pointLights, const std::vector<const SpotLight*>& spotLights)
	{
		static auto nonInstObjectFlagInfo{m_ObjectShader.GetFlagInfo()};
		static auto instObjectFlagInfo{m_ObjectShaderInstanced.GetFlagInfo()};
		static auto nonInstShadowFlagInfo{m_ShadowShader.GetFlagInfo()};
		static auto instShadowFlagInfo{m_ShadowShaderInstanced.GetFlagInfo()};

		nonInstObjectFlagInfo.Clear();
		instObjectFlagInfo.Clear();
		nonInstShadowFlagInfo.Clear();
		instShadowFlagInfo.Clear();

		const auto dirLight{DataManager::DirectionalLight()};

		nonInstObjectFlagInfo["EXISTS_DIRLIGHT"] = dirLight != nullptr;
		instObjectFlagInfo["EXISTS_DIRLIGHT"] = dirLight != nullptr;
		nonInstObjectFlagInfo["DIRLIGHT_SHADOW"] = dirLight != nullptr && dirLight->CastsShadow();
		instObjectFlagInfo["DIRLIGHT_SHADOW"] = dirLight != nullptr && dirLight->CastsShadow();
		nonInstObjectFlagInfo["EXISTS_SPOTLIGHT"] = spotLights.size() != 0ull;
		instObjectFlagInfo["EXISTS_SPOTLIGHT"] = spotLights.size() != 0ull;
		nonInstObjectFlagInfo["EXISTS_POINTLIGHT"] = pointLights.size() != 0ull;
		instObjectFlagInfo["EXISTS_POINTLIGHT"] = pointLights.size() != 0ull;

		auto& nonInstObjectShader{m_ObjectShader.GetPermutation(nonInstObjectFlagInfo)};
		auto& instObjectShader{m_ObjectShaderInstanced.GetPermutation(instObjectFlagInfo)};
		auto& nonInstShadowShader{m_ShadowShader.GetPermutation(nonInstShadowFlagInfo)};
		auto& instShadowShader{m_ShadowShaderInstanced.GetPermutation(instShadowFlagInfo)};

		auto texCount{1};

		nonInstObjectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		instObjectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		nonInstObjectShader.SetUniform("u_CamPos", Camera::Active->Entity.Transform->Position());
		instObjectShader.SetUniform("u_CamPos", Camera::Active->Entity.Transform->Position());

		/* Set up ambient light data */
		nonInstObjectShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());
		instObjectShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (dirLight != nullptr)
		{
			nonInstObjectShader.SetUniform("u_DirLight.direction", dirLight->Direction());
			instObjectShader.SetUniform("u_DirLight.direction", dirLight->Direction());
			nonInstObjectShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
			instObjectShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
			nonInstObjectShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
			instObjectShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());

			if (dirLight->CastsShadow())
			{
				static std::vector<Matrix4> dirLightMatrices;
				static std::vector<float> cascadeFarBounds;

				dirLightMatrices.clear();
				cascadeFarBounds.clear();

				const auto cameraInverseMatrix{camViewMat.Inverse()};
				const auto lightViewMatrix{Matrix4::LookAt(dirLight->Range() * -dirLight->Direction(), Vector3{}, Vector3::Up())};
				const auto cascadeCount{Settings::DirectionalShadowCascadeCount()};

				for (std::size_t i = 0; i < cascadeCount; ++i)
				{
					const auto lightWorldToClip{m_DirLightShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
					dirLightMatrices.push_back(lightWorldToClip);

					nonInstShadowShader.SetUniform("u_WorldToClipMat", lightWorldToClip);
					instShadowShader.SetUniform("u_WorldToClipMat", lightWorldToClip);

					m_DirLightShadowMap.BindForWriting(i);
					m_DirLightShadowMap.Clear();

					nonInstShadowShader.Use();
					for (const auto& [renderable, component] : DataManager::NonInstancedRenderables())
					{
						nonInstShadowShader.SetUniform("u_ModelMat", DataManager::GetMatrices(component->Entity.Transform).first);
						renderable->DrawDepth();
					}

					instShadowShader.Use();
					for (const auto& [renderable, components] : DataManager::InstancedRenderables())
					{
						if (renderable.CastsShadow())
						{
							renderable.DrawDepth();
						}
					}
				}

				m_DirLightShadowMap.UnbindFromWriting();

				for (std::size_t i = 0; i < cascadeCount; ++i)
				{
					const auto viewSpaceBound{m_DirLightShadowMap.CascadeBoundsViewSpace(i)[1]};
					const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
					const auto clipSpaceBoundVector{viewSpaceBoundVector * camProjMat};
					const auto clipSpaceBound{clipSpaceBoundVector[2]};
					cascadeFarBounds.push_back(clipSpaceBound);
				}

				nonInstObjectShader.SetUniform("u_DirLightCascadeCount", static_cast<unsigned>(cascadeCount));
				instObjectShader.SetUniform("u_DirLightCascadeCount", static_cast<unsigned>(cascadeCount));
				nonInstObjectShader.SetUniform("u_DirLightClipMatrices", dirLightMatrices);
				instObjectShader.SetUniform("u_DirLightClipMatrices", dirLightMatrices);
				nonInstObjectShader.SetUniform("u_DirLightCascadeFarBounds", cascadeFarBounds);
				instObjectShader.SetUniform("u_DirLightCascadeFarBounds", cascadeFarBounds);
				static_cast<void>(m_DirLightShadowMap.BindForReading(nonInstObjectShader, texCount));
				texCount = m_DirLightShadowMap.BindForReading(instObjectShader, texCount);
			}
		}

		/* Set up PointLight data */
		nonInstObjectShader.SetUniform("u_PointLightCount", static_cast<int>(pointLights.size()));
		instObjectShader.SetUniform("u_PointLightCount", static_cast<int>(pointLights.size()));
		for (std::size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].position", pointLight->Entity.Transform->Position());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].position", pointLight->Entity.Transform->Position());
			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			nonInstObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
			instObjectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		nonInstObjectShader.SetUniform("u_SpotLightCount", static_cast<int>(spotLights.size()));
		instObjectShader.SetUniform("u_SpotLightCount", static_cast<int>(spotLights.size()));
		for (std::size_t i = 0; i < spotLights.size(); i++)
		{
			const auto& spotLight{spotLights[i]};

			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].position", spotLight->Entity.Transform->Position());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].position", spotLight->Entity.Transform->Position());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].direction", spotLight->Entity.Transform->Forward());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].direction", spotLight->Entity.Transform->Forward());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			nonInstObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
			instObjectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		nonInstObjectShader.Use();
		for (const auto& [renderable, component] : DataManager::NonInstancedRenderables())
		{
			const auto& [modelMat, normalMat]
			{
				DataManager::GetMatrices(component->Entity.Transform)
			};
			nonInstObjectShader.SetUniform("u_ModelMat", modelMat);
			nonInstObjectShader.SetUniform("u_NormalMat", normalMat);
			renderable->DrawShaded(nonInstObjectShader, 0);
		}

		instObjectShader.Use();
		for (const auto& [renderable, components] : DataManager::InstancedRenderables())
		{
			renderable.DrawShaded(instObjectShader, 0);
		}
	}


	void ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		if (const auto& skybox{Camera::Active->Background().skybox}; skybox.has_value())
		{
			static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
			auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

			skyboxShader.Use();
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			DataManager::Skyboxes().find(skybox->AllFilePaths())->first.Draw(skyboxShader);
		}
	}
}
