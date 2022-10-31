#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Core.hpp"
#include "Components.hpp"
#include "Util.hpp"

#include <d3d11.h>
#include <dxgi1_2.h>

#include <wrl/client.h>

#include <memory>
#include <vector>

namespace leopph
{
	class RenderCore
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mBackBufRtv;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mCubeVertShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mCubePixShader;
		Extent2D<u32> mRenderRes;
		f32 mRenderAspectRatio;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mInstanceBuffer;
		UINT mInstanceBufferElementCapacity;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mCbuffer;
		UINT mIndexCount;
		u32 mSyncInterval{ 0 };
		NormalizedViewport mNormViewport{ 0, 0, 1, 1 };

		std::vector<CubeModel const*> mCubeModels;

		UINT const static sInstanceBufferElementSize;
		UINT const static sVertexBufferSlot;
		UINT const static sInstanceBufferSlot;
		UINT const static sSwapChainFlags;
		UINT const static sPresentFlags;
		static RenderCore* sLastInstance;

		static void on_window_resize(RenderCore* self, Extent2D<u32> size);

		RenderCore(Microsoft::WRL::ComPtr<ID3D11Device> device,
				   Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
				   Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain,
				   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufRtv,
				   Microsoft::WRL::ComPtr<ID3D11VertexShader> cubeVertShader,
				   Microsoft::WRL::ComPtr<ID3D11PixelShader> cubePixShader,
				   Extent2D<u32> renderRes,
				   f32 renderAspectRatio,
				   Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer,
				   UINT instanceBufferElementCapacity,
				   Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer,
				   UINT indexCount);


	public:
		LEOPPHAPI [[nodiscard]] static std::unique_ptr<RenderCore> create();

		LEOPPHAPI void present() const;
		LEOPPHAPI bool render();

		[[nodiscard]] u32 get_sync_interval() const;
		void set_sync_interval(u32 interval);

		void register_cube_model(CubeModel const* cubeModel);
		void unregister_cube_model(CubeModel const* cubeModel);

		static [[nodiscard]] RenderCore* get_last_instance();

		LEOPPHAPI [[nodiscard]] ID3D11Device* get_device() const;
		LEOPPHAPI [[nodiscard]] ID3D11DeviceContext* get_immediate_context() const;

		LEOPPHAPI [[nodiscard]] NormalizedViewport const& get_normalized_viewport() const;
		LEOPPHAPI void set_normalized_viewport(NormalizedViewport const& nvp);
	};
}