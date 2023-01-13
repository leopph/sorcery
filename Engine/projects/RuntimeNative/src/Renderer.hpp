#pragma once

#include "Core.hpp"
#include "CubeModelComponent.hpp"
#include "CameraComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "Platform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>


namespace leopph {
	struct EditorCamera {
		Vector3 position;
		Quaternion orientation;
		f32 nearClip;
		f32 farClip;
		f32 fovVertRad;
	};

	class Renderer {
	private:
		struct Resources {
			Microsoft::WRL::ComPtr<ID3D11Device> device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

			Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> swapChainRtv;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> gameRenderTexture;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gameRenderTextureRtv;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gameRenderTextureSrv;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> sceneRenderTexture;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneRenderTextureRtv;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneRenderTextureSrv;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> clearColorVs;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> clearColorPs;
			Microsoft::WRL::ComPtr<ID3D11Buffer> clearColorCbuf;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> cubeVertShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> cubePixShader;

			Microsoft::WRL::ComPtr<ID3D11Buffer> cubeVertBuf;
			Microsoft::WRL::ComPtr<ID3D11Buffer> cubeInstBuf;
			Microsoft::WRL::ComPtr<ID3D11Buffer> cubeIndBuf;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> cubeIa;
			Microsoft::WRL::ComPtr<ID3D11Buffer> matrixCBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer;

			Microsoft::WRL::ComPtr<ID3D11InputLayout> quadIa;
			Microsoft::WRL::ComPtr<ID3D11Buffer> quadVertBuf;
			Microsoft::WRL::ComPtr<ID3D11Buffer> quadIndBuf;

			Microsoft::WRL::ComPtr<ID3D11PixelShader> pbrPs;
			Microsoft::WRL::ComPtr<ID3D11Buffer> materialCBuf;
			Microsoft::WRL::ComPtr<ID3D11Buffer> cameraCBuf;

			std::shared_ptr<Material> defaultMaterial;
			std::shared_ptr<Mesh> defaultMesh;
		};

		auto RecreateGameRenderTextureAndViews(u32 width, u32 height) const -> void;
		auto RecreateSceneRenderTextureAndViews(u32 width, u32 height) const -> void;
		static auto on_window_resize(Renderer* self, Extent2D<u32> size) -> void;

		Resources* mResources{ nullptr };
		UINT mPresentFlags{ 0 };
		UINT mSwapChainFlags{ 0 };
		Extent2D<u32> mGameRes;
		f32 mGameAspect;
		Extent2D<u32> mSceneRes;
		f32 mSceneAspect;
		UINT mInstanceBufferElementCapacity;
		u32 mSyncInterval{ 0 };
		std::vector<CubeModelComponent const*> mCubeModels;
		std::vector<DirectionalLightComponent const*> mDirLights;
		std::vector<SpotLight const*> mSpotLights;
		std::vector<PointLightComponent const*> mPointLights;

	public:
		Renderer() noexcept = default;
		~Renderer() noexcept = default;

		LEOPPHAPI auto StartUp() -> void;
		LEOPPHAPI auto ShutDown() noexcept -> void;

		LEOPPHAPI auto DrawCamera(CameraComponent const* cam) -> void;
		LEOPPHAPI auto DrawGame() -> void;
		LEOPPHAPI auto DrawSceneView(EditorCamera const& cam) -> void;

		[[nodiscard]] LEOPPHAPI auto GetGameResolution() const noexcept -> Extent2D<u32>;
		LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;

		LEOPPHAPI [[nodiscard]] auto GetSceneResolution() const noexcept -> Extent2D<u32>;
		LEOPPHAPI auto SetSceneResolution(Extent2D<u32> resolution) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto GetGameFrame() const noexcept -> ID3D11ShaderResourceView*;
		[[nodiscard]] LEOPPHAPI auto GetSceneFrame() const noexcept -> ID3D11ShaderResourceView*;

		[[nodiscard]] LEOPPHAPI auto GetGameAspectRatio() const noexcept -> f32;
		[[nodiscard]] LEOPPHAPI auto GetSceneAspectRatio() const noexcept -> f32;

		LEOPPHAPI auto BindAndClearSwapChain() const noexcept -> void;

		LEOPPHAPI auto Present() const noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto GetSyncInterval() const noexcept -> u32;
		LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

		LEOPPHAPI auto RegisterCubeModel(CubeModelComponent const* CubeModelComponent) -> void;
		LEOPPHAPI auto UnregisterCubeModel(CubeModelComponent const* CubeModelComponent) -> void;

		[[nodiscard]] LEOPPHAPI auto GetDevice() const noexcept -> ID3D11Device*;
		[[nodiscard]] LEOPPHAPI auto GetImmediateContext() const noexcept -> ID3D11DeviceContext*;

		LEOPPHAPI auto RegisterDirLight(DirectionalLightComponent const* dirLight) -> void;
		LEOPPHAPI auto UnregisterDirLight(DirectionalLightComponent const* dirLight) -> void;

		LEOPPHAPI auto RegisterSpotLight(SpotLight const* spotLight) -> void;
		LEOPPHAPI auto UnregisterSpotLight(SpotLight const* spotLight) -> void;

		LEOPPHAPI auto RegisterPointLight(PointLightComponent const* PointLightComponent) -> void;
		LEOPPHAPI auto UnregisterPointLight(PointLightComponent const* PointLightComponent) -> void;

		[[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() const noexcept -> std::shared_ptr<Material>;
	};
}
