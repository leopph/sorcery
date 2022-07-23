#pragma once

#include "GlCore.hpp"
#include "GlMeshGroup.hpp"
#include "GlSkyboxImpl.hpp"
#include "Matrix.hpp"
#include "Renderer.hpp"
#include "ShaderFamily.hpp"

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
			static std::unique_ptr<GlRenderer> Create();

			[[nodiscard]] RenderObject* CreateRenderObject(MeshGroup const& meshGroup) override;

			void DeleteRenderObject(RenderObject* renderObject) override;

			[[nodiscard]] GlSkyboxImpl* CreateOrGetSkyboxImpl(std::filesystem::path allPaths);

			void DestroySkyboxImpl(GlSkyboxImpl const* skyboxImpl);

		protected:
			// Extract game data per RenderObject per RenderComponent, process them, and output into RenderNodes.
			void ExtractAndProcessInstanceData(std::vector<RenderNode>& out);


			/* ########################
			 * RENDER UTILITY FUNCTIONS
			 * ######################## */

			// Transform the near and far distances of the passed cascades to NDC space.
			// The first values are the near bounds, the second ones are the far bounds.
			// After the call out will contain the same number of elements as cascades.
			void CascadeBoundToNdc(std::span<ShadowCascade const> cascades, std::vector<std::pair<f32, f32>>& out) const;


			/* ##########################
			 * COMMON RESOURCE MANAGEMENT
			 * ########################## */

		private:
			// Creates a VAO and VBO filled with the vertices for a fullscreen quad
			void CreateScreenQuad();

			// Destroys the VAO and VBO of the fullscreen quad
			void DeleteScreenQuad() const;

		protected:
			// Issues a draw call with the fullscreen quad's VAO bound
			void DrawScreenQuad() const;

		private:
			// Creates the common render targets with the given render sizes
			void CreateRenderTargets(GLsizei renderWidth, GLsizei renderHeight);

			// Deletes the common render targets
			void DeleteRenderTargets() const;

			// Creates the shadow cascade maps with the given resolutions
			void CreateDirShadowMaps(std::span<u16 const> resolutions);

			// Deletes the shadow cascade maps
			void DeleteDirShadowMaps() const;

			// Creates count new spot shadow maps with the given resolution and appends them to the existing ones
			void CreateAppendSpotShadowMaps(u16 resolution, u8 count);

			// Deletes the spot shadow maps
			void DeleteSpotShadowMaps() const;

			// Creates count new point shadow maps with the given resolution and appends them to the existing ones
			void CreateAppendPointShadowMaps(u16 resolution, u8 count);

			// Deletes the point shadow maps
			void DeletePointShadowMaps() const;


			/* ##################
			 * RESOURCE CALLBACKS
			 * ################## */

		protected:
			// Recreates common render targets
			void OnRenderResChange(Extent2D renderRes) override;

			// Recreates shadow cascade maps
			void OnDirShadowResChange(std::span<u16 const> resolutions) override;

			// Recreates spot shadow maps
			void OnSpotShadowResChange(u16 resolution) override;

			// Recreates point shadow maps
			void OnPointShadowResChange(u16 resolution) override;

			// Creates additional spot and point shadow maps if needed
			void OnDetermineShadowMapCountRequirements(u8 spot, u8 point) override;


			/* ############
			 * RULE OF FIVE
			 * ############ */

			GlRenderer();

		public:
			GlRenderer(GlRenderer const& other) = delete;
			void operator=(GlRenderer const& other) = delete;

			GlRenderer(GlRenderer&& other) = delete;
			void operator=(GlRenderer&& other) = delete;

			~GlRenderer() noexcept override;


			/* ############
			 * DATA MEMBERS
			 * ############ */

		protected:
			ShaderFamily mDepthShadowShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DepthShadow.shader")};
			ShaderFamily mLinearShadowShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LinearShadow.shader")};
			ShaderFamily mSkyboxShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Skybox.shader")};
			ShaderFamily mGammaCorrectShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GammaCorrect.shader")};
			ShaderFamily mForwardShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Forward.shader")};
			ShaderFamily mTransparencyCompositeShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/TransparencyComposite.shader")};

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
