#pragma once

#include "GlRenderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../CubeShadowMap.hpp"
#include "../GeometryBuffer.hpp"
#include "../RenderBuffer.hpp"
#include "../SpotShadowMap.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderFamily.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <memory>
#include <vector>


namespace leopph::internal
{
	class GlDeferredRenderer final : public GlRenderer
	{
		public:
			GlDeferredRenderer();

			auto Render() -> void override;

		private:
			// Fill the GeometryBuffer with geometry data.
			auto RenderGeometry(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables) -> void;

			// Draw all lights in the RenderBuffer.
			auto RenderLights(const Matrix4& camViewMat,
			                  const Matrix4& camProjMat,
			                  std::span<const RenderableData> renderables,
			                  std::span<const SpotLight*> spotLights,
			                  std::span<const PointLight*> pointLights) -> void;

			// Draw skybox in the empty parts of the RenderBuffer.
			auto RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) -> void;

			// Draws into the dirlight shadow map, binds it to light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderDirShadowMap(const DirectionalLight* dirLight,
			                                      const Matrix4& camViewInvMat,
			                                      const Matrix4& camProjMat,
			                                      std::span<const RenderableData> renderables,
			                                      ShaderProgram& lightShader,
			                                      ShaderProgram& shadowShader,
			                                      GLuint nextTexUnit) const -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderSpotShadowMaps(std::span<const SpotLight* const> spotLights,
			                                        std::span<const RenderableData> renderables,
			                                        ShaderProgram& lightShader,
			                                        ShaderProgram& shadowShader,
			                                        std::size_t numShadows,
			                                        GLuint nextTexUnit) -> GLuint;

			// Draws into the first N shadow maps, binds them to the light shader with the necessary data, and returns the next usable texture unit.
			[[nodiscard]] auto RenderPointShadowMaps(std::span<const PointLight* const> pointLights,
			                                         std::span<const RenderableData> renderables,
			                                         ShaderProgram& lightShader,
			                                         ShaderProgram& shadowShader,
			                                         std::size_t numShadows,
			                                         GLuint nextTexUnit) -> GLuint;


			struct ShadowCount
			{
				bool Directional{false};
				std::size_t Spot{0};
				std::size_t Point{0};
			};


			static auto SetAmbientData(const AmbientLight& light, ShaderProgram& lightShader) -> void;
			static auto SetDirectionalData(const DirectionalLight* dirLight, ShaderProgram& lightShader) -> void;
			static auto SetSpotData(std::span<const SpotLight* const> spotLights, ShaderProgram& lightShader) -> void;
			static auto SetPointData(std::span<const PointLight* const> pointLights, ShaderProgram& lightShader) -> void;
			static auto CountShadows(const DirectionalLight* dirLight, std::span<const SpotLight* const> spotLights, std::span<const PointLight* const> pointLights) -> ShadowCount;

			GeometryBuffer m_GBuffer;
			RenderBuffer m_RenderBuffer;

			ShaderFamily m_ShadowShader;
			ShaderFamily m_CubeShadowShader;

			ShaderFamily m_GeometryShader;
			ShaderFamily m_LightShader;
			ShaderFamily m_SkyboxShader;

			CascadedShadowMap m_DirShadowMap;
			std::vector<std::unique_ptr<SpotShadowMap>> m_SpotShadowMaps;
			std::vector<std::unique_ptr<CubeShadowMap>> m_PointShadowMaps;

			static constexpr int STENCIL_REF{0};
			static constexpr int STENCIL_AND_MASK{1};
	};
}
