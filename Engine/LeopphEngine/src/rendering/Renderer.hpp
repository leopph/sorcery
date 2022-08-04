#pragma once

#include "CubeMesh.hpp"
#include "EventReceiver.hpp"
#include "StaticMaterial.hpp"
#include "PersistentMappedBuffer.h"
#include "QuadMesh.hpp"
#include "RenderingPath.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "StaticMesh.hpp"
#include "StaticMeshComponent.hpp"
#include "StaticMeshData.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"

#include <memory>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	#pragma warning(push)
	#pragma warning(disable: 4324)
	class Renderer final : public EventReceiver<WindowEvent>
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
			StaticMaterial const* material;
			bool isCastingShadow;
			std::vector<std::pair<Matrix4, Matrix4>> matrices;
		};


		struct StaticMeshGroup
		{
			std::vector<std::unique_ptr<StaticMesh>> meshes;
			std::vector<std::unique_ptr<PersistentMappedBuffer>> transformBuf;
		};


		public:
			// Entry point for rendering a frame
			void render();

			u64 create_static_mesh(StaticMeshComponent const* component, std::span<StaticMeshData const> data);
			void register_material(StaticMaterial const* material);
			void unregister_material(StaticMaterial const* material);


		private:
			// Extract all information from global game and config states
			// Returns true if rendering should continue based on the  observed state, false if it should abort.
			[[nodiscard]] bool extract();
			void prepare();
			void update_resources();
			void submit_common_data() const;
			void forward_render() const;
			void deferred_render() const;


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

			Shader mDepthShadowShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DepthShadow.shader"};
			Shader mLinearShadowShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LinearShadow.shader"};
			Shader mSkyboxShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Skybox.shader"};
			Shader mForwardShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Forward.shader"};
			Shader mTransparencyCompositeShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/TransparencyComposite.shader"};
			Shader mGammaCorrectShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GammaCorrect.shader"};

			QuadMesh mQuadMesh;
			CubeMesh mCubeMesh;


			u64 mFrameCount{0};

			std::unordered_map<StaticMaterial const*, PersistentMappedBuffer> mMaterialBuffers;
	};
	#pragma warning(pop)
}
