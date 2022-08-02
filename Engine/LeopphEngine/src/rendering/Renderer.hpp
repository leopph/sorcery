#pragma once

#include "CubeMesh.hpp"
#include "EventReceiver.hpp"
#include "QuadMesh.hpp"
#include "RenderingPath.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "StaticMesh.hpp"
#include "StaticMeshComponent.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"

#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	#pragma warning(push)
	#pragma warning(disable: 4324)
	class Renderer : public EventReceiver<WindowEvent>
	{
		struct ResourceUpdateFlags
		{
			bool renderRes : 1 = false;
		};


		struct UboAmbientLight
		{
			alignas(16) Vector3 intensity;
		};


		struct UboLight
		{
			alignas(16) Vector3 color;
			f32 intensity;
		};


		struct UboDirLight
		{
			UboLight lightBase;
			alignas(16) Vector3 direction;
			u32 shadow;
		};


		struct UboSpotLight
		{
			UboLight lightBase;
			alignas(16) Vector3 position;
			f32 range;
			alignas(16) Vector3 direction;
			f32 innerCos;
			f32 outerCos;
		};


		struct UboPointLight
		{
			UboLight lightBase;
			alignas(16) Vector3 position;
			f32 range;
		};


		struct UboCameraData
		{
			Matrix4 viewMat;
			Matrix4 viewMatInv;
			Matrix4 projMat;
			Matrix4 projMatInv;
			Matrix4 viewProjMat;
			Matrix4 viewProjMatInv;
			alignas(16) Vector3 position;
		};


		struct ScreenData
		{
			u32 renderWidth;
			u32 renderHeight;
			u32 width;
			u32 height;
			f32 gamma;
		};


		struct PingPongFramebuffer
		{
			u32 framebuffer;
			u32 colorBuffer;
			u32 depthStencilBuffer;
			u32 width;
			u32 height;
		};


		struct UniformBuffer
		{
			u32 name;
			u32 size;
			u8* mapping;
		};


		struct RenderNode
		{
			u32 vao;
			Material const* material;
			bool isCastingShadow;
			std::vector<std::pair<Matrix4, Matrix4>> matrices;
		};


		public:
			// Entry point for rendering a frame
			void render();

			// TODO system for creating, handing out, and deleting StaticMeshes 
			void register_static_mesh(StaticMeshComponent const* component, StaticMesh const* mesh);
			void unregister_static_mesh(StaticMeshComponent const* component);

			u32 request_buffer();
			void release_buffer(u32 buffer);

			u32 request_vao();
			void release_vao(u32 vao);


		private:
			// Extract all information from global game and config states
			// Returns true if rendering should continue based on the  observed state, false if it should abort.
			[[nodiscard]] bool extract();
			void prepare();
			void update_resources();
			void submit_common_data() const;
			void forward_render() const;
			void deferred_render() const;

			void draw_screen_quad() const;
			static void draw_static_mesh(u32 vao, u32 numIndices);

			void clean_up_released_resources();


			void OnEventReceived(EventReceiver<WindowEvent>::EventParamType) override;


			//void calculate_shadow_cascades(std::vector<ShadowCascade>& out);


		public:
			Renderer();

			Renderer(Renderer const& other) = delete;
			Renderer& operator=(Renderer const& other) = delete;

			Renderer(Renderer&& other) noexcept = delete;
			Renderer& operator=(Renderer&& other) noexcept = delete;

			~Renderer() override;


		private:
			UboCameraData mCamData;
			ScreenData mScreenData;

			UboAmbientLight mAmbientLightData;
			std::optional<UboDirLight> mDirLightData;
			std::vector<UboSpotLight> mSpotLightData;
			std::vector<UboPointLight> mPointLightData;

			std::vector<RenderNode> mRenderNodes;
			std::span<RenderNode const> mActiveRenderNodes;

			std::array<PingPongFramebuffer, 2> mPingPongBuffers{};

			// How many actual buffer objects we create for each buffer.
			// For example 3 means every uniform buffer is triple buffered.
			u8 constexpr static NUM_UNIFORM_BUFFERS{3};

			std::array<UniformBuffer, NUM_UNIFORM_BUFFERS> mCameraBuffers{};
			std::array<UniformBuffer, NUM_UNIFORM_BUFFERS> mLightingBuffers{};

			ResourceUpdateFlags mResUpdateFlags;
			RenderingPath mRenderingPath;

			Shader mDepthShadowShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DepthShadow.shader"};
			Shader mLinearShadowShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LinearShadow.shader"};
			Shader mSkyboxShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Skybox.shader"};
			Shader mGammaCorrectShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GammaCorrect.shader"};
			Shader mForwardShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Forward.shader"};
			Shader mTransparencyCompositeShaderFamily{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/TransparencyComposite.shader"};

			QuadMesh mQuadMesh;
			CubeMesh mCubeMesh;

			std::unordered_map<StaticMesh const*, std::vector<StaticMeshComponent const*>> mStaticMeshes;

			u64 mFrameCount{0};
	};
	#pragma warning(pop)
}
