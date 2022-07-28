#pragma once

#include "EventReceiver.hpp"
#include "Light.hpp"
#include "RenderingPath.hpp"
#include "ShaderFamily.hpp"
#include "SkyboxImpl.hpp"
#include "StaticMeshGroup.hpp"
#include "StaticMeshGroupComponent.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"

#include <unordered_map>
#include <vector>


namespace leopph::internal
{
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


		struct ScreenQuad
		{
			u32 vao;
			u32 vbo;
		};


		public:
			// Entry point for rendering a frame
			void render();

			void register_static_mesh_group(StaticMeshGroupComponent const* component, StaticMeshGroup const* model);
			void unregister_static_mesh_group(StaticMeshGroupComponent const* component);

			[[nodiscard]] SkyboxImpl* create_or_get_skybox_impl(std::filesystem::path allPaths);
			void destroy_skybox_impl(SkyboxImpl const* skyboxImpl);


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


			void OnEventReceived(EventReceiver<WindowEvent>::EventParamType) override;


			//void calculate_shadow_cascades(std::vector<ShadowCascade>& out);


		public:
			Renderer();

			Renderer(Renderer const& other) = delete;
			Renderer& operator=(Renderer const& other) = delete;

			Renderer(Renderer&& other) noexcept = delete;
			Renderer& operator=(Renderer&& other) noexcept = delete;

			~Renderer() noexcept override;


		private:
			UboCameraData mCamData;
			ScreenData mScreenData;

			UboAmbientLight mAmbientLightData;
			std::optional<UboDirLight> mDirLightData;
			std::vector<UboSpotLight> mSpotLightData;
			std::vector<UboPointLight> mPointLightData;

			std::array<PingPongFramebuffer, 2> mPingPongBuffers{};

			// How many actual buffer objects we create for each buffer.
			// For example 3 means every uniform buffer is triple buffered.
			u8 constexpr static NUM_UNIFORM_BUFFERS{3};

			// Modulo ++ with NUM_UNIFORM_BUFFERS
			u8 mUboIndex{0};

			std::array<UniformBuffer, NUM_UNIFORM_BUFFERS> mCameraBuffers{};
			std::array<UniformBuffer, NUM_UNIFORM_BUFFERS> mLightingBuffers{};

			ScreenQuad mScreenQuad;


			ResourceUpdateFlags mResUpdateFlags;
			RenderingPath mRenderingPath;

			ShaderFamily mDepthShadowShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/DepthShadow.shader")};
			ShaderFamily mLinearShadowShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/LinearShadow.shader")};
			ShaderFamily mSkyboxShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Skybox.shader")};
			ShaderFamily mGammaCorrectShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/GammaCorrect.shader")};
			ShaderFamily mForwardShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/Forward.shader")};
			ShaderFamily mTransparencyCompositeShaderFamily{make_shader_family("C:/Dev/LeopphEngine/Engine/LeopphEngine/src/rendering/shaders/glsl/TransparencyComposite.shader")};

			std::unordered_map<StaticMeshGroupComponent const*, StaticMeshGroup const*> mStaticModels;
			std::vector<std::unique_ptr<SkyboxImpl>> mSkyboxes;
	};
}
