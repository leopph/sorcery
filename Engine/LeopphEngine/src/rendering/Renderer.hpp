#pragma once

#include "AABB.hpp"
#include "Camera.hpp"
#include "CubeMesh.hpp"
#include "DirectionalLight.hpp"
#include "EventReceiverHandle.hpp"
#include "Framebuffer.hpp"
#include "GlContext.hpp"
#include "PersistentMappedBuffer.hpp"
#include "PointLight.hpp"
#include "QuadMesh.hpp"
#include "Shader.hpp"
#include "SpotLight.hpp"
#include "StaticMaterial.hpp"
#include "StaticMesh.hpp"
#include "StaticMeshComponent.hpp"
#include "StaticModelData.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace leopph::internal
{
	class Renderer : OpenGlContext
	{
		struct ResourceUpdateFlags
		{
			bool renderRes : 1{false};
		};


		enum class UboLightType : i32
		{
			Directional = 0,
			Spot = 1,
			Point = 2
		};


		struct UboLightData
		{
			alignas(16) Vector3 color;
			f32 intensity;
			Vector3 position;
			f32 range;
			Vector3 direction;
			f32 innerCos;
			f32 outerCos;
			i32 type;
		};


		constexpr static auto NUM_MAX_LIGHTS = 8;


		struct UboPerFrameData
		{
			Vector3 ambient;
			i32 lightCount;
			std::array<UboLightData, NUM_MAX_LIGHTS> lights;
		};


		struct UboPerCameraData
		{
			Matrix4 viewMat;
			Matrix4 viewMatInv;
			Matrix4 projMat;
			Matrix4 projMatInv;
			Matrix4 viewProjMat;
			Matrix4 viewProjMatInv;
			Vector3 position;
		};


		struct MeshNode
		{
			std::shared_ptr<StaticMesh> mesh;
			std::vector<std::pair<Matrix4, Matrix4>> matrices; // TODO accumulate per view matrices, put them in the buffer once, store only indices, and use glDrawElementsBaseInstance
		};


		struct MaterialNode
		{
			std::shared_ptr<StaticMaterial const> material;
			std::vector<MeshNode> meshNodes;
		};


		struct ViewData
		{
			Extent<u32> viewport;
			UboPerCameraData uboPerCameraData;
			std::vector<MaterialNode> materialNodes;
		};


		struct FrameData
		{
			UboPerFrameData uboPerFrameData;
			std::vector<ViewData> perViewData;
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

			void register_mesh_for_material(std::shared_ptr<StaticMaterial const> const& material, std::shared_ptr<StaticMesh> mesh);
			void unregister_mesh_for_material(std::shared_ptr<StaticMaterial const> const& material, std::shared_ptr<StaticMesh> const& mesh);

		private:
			void create_ping_pong_buffers();
			void delete_ping_pong_buffers();


			//void calculate_shadow_cascades(std::vector<ShadowCascade>& out);


		public:
			Renderer();

			Renderer(Renderer const& other) = delete;
			Renderer(Renderer&& other) = delete;

			Renderer& operator=(Renderer const& other) = delete;
			Renderer& operator=(Renderer&& other) = delete;

			~Renderer();


		private:
			ResourceUpdateFlags mResUpdateFlags;
			EventReceiverHandle<WindowEvent> mWindowEventReceiver{
				[this](auto const&)
				{
					mResUpdateFlags.renderRes = true;
				}
			};


			//Shader mDepthShadowShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DepthShadow.shader"};
			//Shader mLinearShadowShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LinearShadow.shader"};
			//Shader mSkyboxShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Skybox.shader"};
			Shader mForwardShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Forward.shader"};
			//Shader mTransparencyCompositeShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/TransparencyComposite.shader"};
			//Shader mGammaCorrectShader{"C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GammaCorrect.shader"};

			QuadMesh mQuadMesh;
			CubeMesh mCubeMesh;

			u64 mFrameCount{0};

			std::array<Framebuffer, 2> mPingPongBuffers;
			PersistentMappedBuffer mPerFrameUbo{sizeof UboPerFrameData};
			PersistentMappedBuffer mPerCameraUbo{sizeof UboPerCameraData};

			std::unordered_map<std::shared_ptr<StaticMaterial const>, std::vector<std::shared_ptr<StaticMesh>>> mMaterialsToMeshes;

			FrameData mFrameData{};

			std::vector<DirectionalLight const*> mDirLights;
			std::vector<SpotLight const*> mSpotLights;
			std::vector<PointLight const*> mPointLights;
			std::vector<Camera const*> mCameras;
	};
}
