#pragma once

#include "Camera.hpp"
#include "CubeMesh.hpp"
#include "DirectionalLight.hpp"
#include "EventReceiver.hpp"
#include "PersistentMappedBuffer.hpp"
#include "PointLight.hpp"
#include "QuadMesh.hpp"
#include "RenderingPath.hpp"
#include "Shader.hpp"
#include "SpotLight.hpp"
#include "StaticModel.hpp"
#include "StaticModelComponent.hpp"
#include "StaticModelData.hpp"
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


		struct CameraData
		{
			Extent<u32> extent;
			Vector3 position;
			Matrix4 viewMatrix;
			Matrix4 projectionMatrix;
		};


		public:
			// Entry point for rendering a frame
			void render();
			

			void register_dir_light(DirectionalLight const* dirLight);
			void unregister_dir_light(DirectionalLight const* dirLight);

			void register_spot_light(SpotLight const* spotLight);
			void unregister_spot_light(SpotLight const* spotLight);

			void register_point_light(PointLight const* pointLight);
			void unregister_point_light(PointLight const* pointLight);

			void register_camera(Camera const* camera);
			void unregister_camera(Camera const* camera);


		private:
			static void extract_camera_data(Camera const& camera, std::vector<CameraData>& out);

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

			Renderer(Renderer&& other) = delete;
			Renderer& operator=(Renderer&& other) = delete;

			~Renderer() override;


		private:
			std::vector<CameraData> mCameraData;

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

			std::unordered_map<StaticMaterial const*, std::vector<StaticMesh const*>> mStaticMeshes;

			std::vector<DirectionalLight const*> mDirLights;
			std::vector<SpotLight const*> mSpotLights;
			std::vector<PointLight const*> mPointLights;
			std::vector<Camera const*> mCameras;
	};
	#pragma warning(pop)
}
