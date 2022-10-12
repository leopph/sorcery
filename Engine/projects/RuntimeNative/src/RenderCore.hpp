#pragma once

#include "Extent2D.hpp"
#include "Core.hpp"
#include "Window.hpp"

#include <d3d11.h>
#include <dxgi1_2.h>

#include <wrl/client.h>

#include <memory>

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
		Extent2D mRenderRes;
		f32 mRenderAspectRatio;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mInstanceBuffer;
		UINT mInstanceBufferElementCapacity;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mCbuffer;
		UINT mIndexCount;

		UINT const static sInstanceBufferElementSize;
		UINT const static sVertexBufferSlot;
		UINT const static sInstanceBufferSlot;

		static void on_window_resize(RenderCore* self, Extent2D size);

		RenderCore(Microsoft::WRL::ComPtr<ID3D11Device> device,
				   Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
				   Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain,
				   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufRtv,
				   Microsoft::WRL::ComPtr<ID3D11VertexShader> cubeVertShader,
				   Microsoft::WRL::ComPtr<ID3D11PixelShader> cubePixShader,
				   Extent2D renderRes,
				   f32 renderAspectRatio,
				   Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer,
				   UINT instanceBufferElementCapacity,
				   Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer,
				   UINT indexCount,
				   Window& window);

	public:
		LEOPPHAPI [[nodiscard]] static std::unique_ptr<RenderCore> Create(Window& window);

		LEOPPHAPI bool render();
	};
}