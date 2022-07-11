#pragma once

#include "AmbientLight.hpp"
#include "DirLight.hpp"
#include "Matrix.hpp"
#include "PointLight.hpp"
#include "Renderer.hpp"
#include "SpotLight.hpp"
#include "rendering/gl/GlCascadedShadowMap.hpp"
#include "rendering/gl/GlCubeShadowMap.hpp"
#include "rendering/gl/GlMeshGroup.hpp"
#include "rendering/gl/GlRenderBuffer.hpp"
#include "rendering/gl/GlSkyboxImpl.hpp"
#include "rendering/gl/GlSpotShadowMap.hpp"
#include "rendering/gl/GlTransparencyBuffer.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

#include <filesystem>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	class GlRenderer : public Renderer
	{
		protected:
			struct RenderNode;
			struct ShadowCount;

		public:
			// Initializes OpenGL and constructs a renderer for the selected rendering pipeline.
			static auto Create() -> std::unique_ptr<GlRenderer>;

			[[nodiscard]] auto CreateRenderObject(MeshGroup const& meshGroup) -> RenderObject* override;
			auto DeleteRenderObject(RenderObject* renderObject) -> void override;

			[[nodiscard]] auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> GlSkyboxImpl*;
			auto DestroySkyboxImpl(GlSkyboxImpl const* skyboxImpl) -> void;

		protected:
			// Extract game data per RenderObject per RenderComponent, process them, and output into RenderNodes.
			auto ExtractAndProcessInstanceData(std::vector<RenderNode>& out) -> void;

			// Fills out with at most Settings::MaxSpotLights number of SpotLight instances based on distance from the current Camera.
			static auto ExtractSpotLightsCurrentCamera(std::vector<SpotLight const*>& out) -> void;

			// Fills out with at most Settings::MaxSpotLights number of SpotLight instances based on distance from the current Camera.
			static auto ExtractPointLightsCurrentCamera(std::vector<PointLight const*>& out) -> void;

			// Returns a collection of cascade far bounds in NDC.
			[[nodiscard]] static auto CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<GlCascadedShadowMap::CascadeBounds const> cascadeBounds) -> std::span<float const>;

			static auto SetAmbientData(AmbientLight const& light, ShaderProgram& lightShader) -> void;
			static auto SetDirectionalData(DirectionalLight const* dirLight, ShaderProgram& shader) -> void;
			static auto SetSpotData(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetSpotDataIgnoreShadow(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetPointData(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto SetPointDataIgnoreShadow(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto CountShadows(DirectionalLight const* dirLight, std::span<SpotLight const* const> spotLights, std::span<PointLight const* const> pointLights) -> ShadowCount;

			auto ApplyGammaCorrection() -> void;
			auto Present() const -> void;


		private:
			auto CreateScreenQuad() -> void;
		protected:
			auto DrawScreenQuad() const -> void;
		private:
			auto DeleteScreenQuad() const -> void;


		protected:
			GlRenderer();

		public:
			GlRenderer(GlRenderer const& other) = delete;
			auto operator=(GlRenderer const& other) -> void = delete;

			GlRenderer(GlRenderer&& other) = delete;
			auto operator=(GlRenderer&& other) -> void = delete;

			~GlRenderer() noexcept override;

		protected:
			GlRenderBuffer m_RenderBuffer;
			GlRenderBuffer m_GammaCorrectedBuffer;
			GlTransparencyBuffer m_TransparencyBuffer{&m_RenderBuffer.DepthStencilBuffer()};

			GlCascadedShadowMap m_DirShadowMap;
			std::vector<std::unique_ptr<GlSpotShadowMap>> m_SpotShadowMaps;
			std::vector<std::unique_ptr<GlCubeShadowMap>> m_PointShadowMaps;

			ShaderFamily m_ShadowShader
			{
				{
					{ShaderFamily::DepthShadowVertSrc, ShaderType::Vertex}
				}
			};
			ShaderFamily m_CubeShadowShader
			{
				{
					{ShaderFamily::LinearShadowVertSrc, ShaderType::Vertex},
					{ShaderFamily::LinearShadowFragSrc, ShaderType::Fragment}
				}
			};
			ShaderFamily m_SkyboxShader
			{
				{
					{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
					{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
				}
			};
			ShaderFamily m_GammaCorrectShader
			{
				{
					{ShaderFamily::Pos2DPassthroughVertSrc, ShaderType::Vertex},
					{ShaderFamily::GammaCorrectFragSrc, ShaderType::Fragment}
				}
			};

		private:
			// Returns true if left is closer to the current camera then right.
			static auto CompareLightsByDistToCurrentCam(Light const* left, Light const* right) -> bool;

			// Per RenderObject arrays of matrices used. Instanced RenderNodes refer to them.
			std::unordered_map<GlMeshGroup*, std::vector<std::pair<Matrix4, Matrix4>>> m_InstancedMatrixCache;

			// Global cache for all non-instanced RenderNode matrices.
			std::vector<std::pair<Matrix4, Matrix4>> m_NonInstancedMatrixCache;

			std::vector<std::unique_ptr<GlMeshGroup>> m_RenderObjects;

			std::vector<std::unique_ptr<GlSkyboxImpl>> m_SkyboxImpls;

			GLuint m_ScreenQuadVao{};
			GLuint m_ScreenQuadVbo{};
	};


	struct GlRenderer::RenderNode
	{
		GlMeshGroup* RenderObject;
		std::span<std::pair<Matrix4, Matrix4> const> Instances; // matrices are in OpenGL buffer format.
		bool CastsShadow;
	};


	struct GlRenderer::ShadowCount
	{
		bool Directional{false};
		std::size_t Spot{0};
		std::size_t Point{0};
	};
}
