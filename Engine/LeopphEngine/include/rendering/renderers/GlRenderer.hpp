#pragma once

#include "AmbientLight.hpp"
#include "DirLight.hpp"
#include "Matrix.hpp"
#include "PointLight.hpp"
#include "Renderer.hpp"
#include "SpotLight.hpp"
#include "rendering/CascadedShadowMap.hpp"
#include "rendering/GlMeshGroup.hpp"
#include "rendering/SkyboxImpl.hpp"

#include <filesystem>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	class GlRenderer : public Renderer
	{
		public:
			// Constructs an OpenGL renderer for the selected rendering pipeline.
			static auto Create() -> std::unique_ptr<GlRenderer>;

			[[nodiscard]] auto CreateRenderObject(MeshGroup const& meshGroup) -> RenderObject* override;
			auto DeleteRenderObject(RenderObject* renderObject) -> void override;

			[[nodiscard]] auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;
			auto DestroySkyboxImpl(SkyboxImpl const* skyboxImpl) -> void;

			GlRenderer(GlRenderer const& other) = delete;
			auto operator=(GlRenderer const& other) -> void = delete;

			GlRenderer(GlRenderer&& other) = delete;
			auto operator=(GlRenderer&& other) -> void = delete;

			~GlRenderer() noexcept override = default;

		protected:
			GlRenderer();


			struct RenderNode
			{
				GlMeshGroup* RenderObject;
				std::span<std::pair<Matrix4, Matrix4> const> Instances; // matrices are in OpenGL buffer format.
				bool CastsShadow;
			};


			struct ShadowCount
			{
				bool Directional{false};
				std::size_t Spot{0};
				std::size_t Point{0};
			};


			// Extract game data per RenderObject per RenderComponent, process them, and output into RenderNodes.
			auto ExtractAndProcessInstanceData(std::vector<RenderNode>& out) -> void;

			// Fills out with at most Settings::MaxSpotLights number of SpotLight instances based on distance from the current Camera.
			static auto ExtractSpotLightsCurrentCamera(std::vector<SpotLight const*>& out) -> void;

			// Fills out with at most Settings::MaxSpotLights number of SpotLight instances based on distance from the current Camera.
			static auto ExtractPointLightsCurrentCamera(std::vector<PointLight const*>& out) -> void;

			// Returns a collection of cascade far bounds in NDC.
			[[nodiscard]] static auto CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<CascadedShadowMap::CascadeBounds const> cascadeBounds) -> std::span<float const>;

			static auto SetAmbientData(AmbientLight const& light, ShaderProgram& lightShader) -> void;
			static auto SetDirectionalData(DirectionalLight const* dirLight, ShaderProgram& shader) -> void;
			static auto SetSpotData(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetSpotDataIgnoreShadow(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetPointData(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto SetPointDataIgnoreShadow(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto CountShadows(DirectionalLight const* dirLight, std::span<SpotLight const* const> spotLights, std::span<PointLight const* const> pointLights) -> ShadowCount;

		private:
			// Returns true if left is closer to the current camera then right.
			static auto CompareLightsByDistToCurrentCam(Light const* left, Light const* right) -> bool;

			// Per RenderObject arrays of matrices used. Instanced RenderNodes refer to them.
			std::unordered_map<GlMeshGroup*, std::vector<std::pair<Matrix4, Matrix4>>> m_InstancedMatrixCache;

			// Global cache for all non-instanced RenderNode matrices.
			std::vector<std::pair<Matrix4, Matrix4>> m_NonInstancedMatrixCache;

			std::vector<std::unique_ptr<GlMeshGroup>> m_RenderObjects;

			std::vector<std::unique_ptr<SkyboxImpl>> m_SkyboxImpls;
	};
}
