#pragma once

#include "Matrix.hpp"
#include "Renderer.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/gl/GlMeshGroup.hpp"
#include "rendering/gl/GlSkyboxImpl.hpp"
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


			struct PingPongBuffer
			{
				GLuint framebuffer;
				GLuint colorAttachment;
			};


			struct TransparencyBuffer
			{
				GLuint framebuffer;
				GLuint accumAttachment;
				GLuint revealAttachment;
			};


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


			/* ########################
			 * RENDER UTILITY FUNCTIONS
			 * ######################## */

			// Transform the near and far distances of the passed cascades to NDC space.
			// The first values are the near bounds, the second ones are the far bounds.
			// After the call out will contain the same number of elements as cascades.
			auto CascadeBoundToNdc(std::span<ShadowCascade const> cascades, std::vector<std::pair<f32, f32>>& out) const -> void;


			/* ##########################
			 * COMMON RESOURCE MANAGEMENT
			 * ########################## */

		private:
			// Creates a VAO and VBO filled with the vertices for a fullscreen quad
			auto CreateScreenQuad() -> void;

			// Destroys the VAO and VBO of the fullscreen quad
			auto DeleteScreenQuad() const -> void;

		protected:
			// Issues a draw call with the fullscreen quad's VAO bound
			auto DrawScreenQuad() const -> void;

		private:
			// Creates the common render targets with the given render sizes
			auto CreateRenderTargets(GLsizei renderWidth, GLsizei renderHeight) -> void;

			// Deletes the common render targets
			auto DeleteRenderTargets() const -> void;

			// Creates the shadow cascade maps with the given resolutions
			auto CreateDirShadowMaps(std::span<u16 const> resolutions) -> void;

			// Deletes the shadow cascade maps
			auto DeleteDirShadowMaps() const -> void;

			// Creates count new spot shadow maps with the given resolution and appends them to the existing ones
			auto CreateAppendSpotShadowMaps(u16 resolution, u8 count) -> void;

			// Deletes the spot shadow maps
			auto DeleteSpotShadowMaps() const -> void;

			// Creates count new point shadow maps with the given resolution and appends them to the existing ones
			auto CreateAppendPointShadowMaps(u16 resolution, u8 count) -> void;

			// Deletes the point shadow maps
			auto DeletePointShadowMaps() const -> void;


			/* ##################
			 * RESOURCE CALLBACKS
			 * ################## */

		protected:
			// Recreates common render targets
			auto OnRenderResChange(Extent2D renderRes) -> void override;

			// Recreates shadow cascade maps
			auto OnDirShadowResChange(std::span<u16 const> resolutions) -> void override;

			// Recreates spot shadow maps
			auto OnSpotShadowResChange(u16 resolution) -> void override;

			// Recreates point shadow maps
			auto OnPointShadowResChange(u16 resolution) -> void override;

			// Creates additional spot and point shadow maps if needed
			auto OnDetermineShadowMapCountRequirements(u8 spot, u8 point) -> void override;


			/* ############
			 * RULE OF FIVE
			 * ############ */

			GlRenderer();

		public:
			GlRenderer(GlRenderer const& other) = delete;
			auto operator=(GlRenderer const& other) -> void = delete;

			GlRenderer(GlRenderer&& other) = delete;
			auto operator=(GlRenderer&& other) -> void = delete;

			~GlRenderer() noexcept override;


			/* ############
			 * DATA MEMBERS
			 * ############ */

		protected:
			ShaderFamily m_ShadowShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex}
				}
			};

			ShaderFamily m_CubeShadowShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

			ShaderFamily m_SkyboxShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

			ShaderFamily m_GammaCorrectShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

			ShaderFamily m_ForwardObjectShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

			ShaderFamily m_TranspCompositeShader
			{
				{
					{shadersources::COMPOSITE_FRAG, ShaderType::Vertex},
					{shadersources::COMPOSITE_FRAG, ShaderType::Fragment}
				}
			};

		private:
			// Per RenderObject arrays of matrices used. Instanced RenderNodes refer to them.
			std::unordered_map<GlMeshGroup*, std::vector<std::pair<Matrix4, Matrix4>>> m_InstancedMatrixCache;

			// Global cache for all non-instanced RenderNode matrices.
			std::vector<std::pair<Matrix4, Matrix4>> m_NonInstancedMatrixCache;

			std::vector<std::unique_ptr<GlMeshGroup>> m_RenderObjects;

			std::vector<std::unique_ptr<GlSkyboxImpl>> m_SkyboxImpls;

			GLuint m_ScreenQuadVao{};
			GLuint m_ScreenQuadVbo{};

		protected:
			std::array<PingPongBuffer, 2> m_PingPongBuffers;
			TransparencyBuffer m_TransparencyBuffer;
			GLuint m_SharedDepthStencilBuffer;

			std::vector<GLuint> m_DirShadowMapFramebuffers;
			std::vector<GLuint> m_DirShadowMapDepthAttachments;

			std::vector<GLuint> m_SpotShadowMapFramebuffers;
			std::vector<GLuint> m_SpotShadowMapDepthAttachments;

			std::vector<GLuint> m_PointShadowMapFramebuffers;
			std::vector<GLuint> m_PointShadowMapColorAttachments;
			std::vector<GLuint> m_PointShadowMapDepthAttachments;
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
