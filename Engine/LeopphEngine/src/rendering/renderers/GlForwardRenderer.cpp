#include "rendering/renderers/GlForwardRenderer.hpp"

#include "Camera.hpp"
#include "DataManager.hpp"
#include "Logger.hpp"
#include "Matrix.hpp"
#include "Settings.hpp"
#include "Window.hpp"
#include "rendering/opengl/OpenGl.hpp"

#include <string>
#include <utility>


namespace leopph::internal
{
	GlForwardRenderer::GlForwardRenderer() :
		m_ObjectShader
		{
			{
				{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
			}
		},
		m_ShadowShader
		{
			{
				{ShaderFamily::DepthShadowVertSrc, ShaderType::Vertex}
			}
		},

		m_SkyboxShader
		{
			{
				{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
				{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
			}
		},
		m_TranspCompositeShader
		{
			{
				{ShaderFamily::TranspCompositeVertSrc, ShaderType::Vertex},
				{ShaderFamily::TranspCompositeFragSrc, ShaderType::Fragment}
			}
		},
		m_TransparencyBuffer{&m_RenderBuffer.DepthStencilBuffer()}
	{
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}


	auto GlForwardRenderer::Render() -> void
	{
		/* We don't render if there is no camera to use */
		if (Camera::Current() == nullptr)
		{
			return;
		}

		static std::vector<RenderableData> renderables;
		static std::vector<SpotLight const*> spotLights;
		static std::vector<PointLight const*> pointLights;

		renderables.clear();
		CollectRenderables(renderables);
		spotLights.clear();
		CollectSpotLights(spotLights);
		pointLights.clear();
		CollectPointLights(pointLights);

		auto const& dirLight{DataManager::Instance().DirectionalLight()};

		auto const camViewMat{Camera::Current()->ViewMatrix()};
		auto const camProjMat{Camera::Current()->ProjectionMatrix()};

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		m_RenderBuffer.Clear();
		m_RenderBuffer.BindForWriting();
		RenderOpaque(camViewMat, camProjMat, renderables, dirLight, spotLights, pointLights);
		RenderSkybox(camViewMat, camProjMat);
		RenderTransparent(camViewMat, camProjMat, renderables, dirLight, spotLights, pointLights);
		Compose();

		auto const window = Window::Instance();
		glBlitNamedFramebuffer(m_RenderBuffer.Framebuffer(), 0, 0, 0, m_RenderBuffer.Width(), m_RenderBuffer.Height(), 0, 0, window->Width(), window->Height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}


	auto GlForwardRenderer::RenderOpaque(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderableData> const& renderables, DirectionalLight const* dirLight, std::vector<SpotLight const*> const& spotLights, std::vector<PointLight const*> const& pointLights) -> void
	{
		m_ObjectShader.Clear();
		m_ObjectShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_ObjectShader["DIRLIGHT_SHADOW"] = std::to_string(false); // temporary
		m_ObjectShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_ObjectShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(0); // temporary
		m_ObjectShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_ObjectShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(0); // temporary
		m_ObjectShader["TRANSPARENT"] = std::to_string(false);

		auto& shader{m_ObjectShader.GetPermutation()};

		shader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		shader.SetUniform("u_CamPos", Camera::Current()->Owner()->Transform()->Position());

		SetAmbientData(AmbientLight::Instance(), shader);
		SetDirectionalData(dirLight, shader);
		SetSpotDataIgnoreShadow(spotLights, shader); // temporary
		SetPointDataIgnoreShadow(pointLights, shader); // temporary

		shader.Use();

		for (auto const& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(shader, 0, false);
		}
	}


	auto GlForwardRenderer::RenderTransparent(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderableData> const& renderables, DirectionalLight const* dirLight, std::vector<SpotLight const*> const& spotLights, std::vector<PointLight const*> const& pointLights) -> void
	{
		m_ObjectShader.Clear();
		m_ObjectShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_ObjectShader["DIRLIGHT_SHADOW"] = std::to_string(false);
		m_ObjectShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_ObjectShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(0);
		m_ObjectShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_ObjectShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(0);
		m_ObjectShader["TRANSPARENT"] = std::to_string(true);
		auto& transpObjectShader{m_ObjectShader.GetPermutation()};

		transpObjectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		transpObjectShader.SetUniform("u_CamPos", Camera::Current()->Owner()->Transform()->Position());

		SetAmbientData(AmbientLight::Instance(), transpObjectShader);
		SetDirectionalData(dirLight, transpObjectShader);
		SetSpotDataIgnoreShadow(spotLights, transpObjectShader);
		SetPointDataIgnoreShadow(pointLights, transpObjectShader);

		transpObjectShader.Use();
		m_TransparencyBuffer.Clear();
		m_TransparencyBuffer.BindForWriting();

		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

		for (auto const& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(transpObjectShader, 0, true);
		}
	}


	auto GlForwardRenderer::Compose() -> void
	{
		auto& compositeShader{m_TranspCompositeShader.GetPermutation()};
		compositeShader.Use();

		m_TransparencyBuffer.BindForReading(compositeShader, 0);
		m_RenderBuffer.BindForWriting();

		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

		m_ScreenQuad.Draw();
	}


	auto GlForwardRenderer::RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void
	{
		if (auto const& background{Camera::Current()->Background()}; std::holds_alternative<Skybox>(background))
		{
			auto& skyboxShader{m_SkyboxShader.GetPermutation()};
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			skyboxShader.Use();

			DataManager::Instance().CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(skyboxShader);
		}
	}
}
