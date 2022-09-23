#pragma once

#include "Api.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

#include "Window.hpp"


namespace leopph
{
	class Renderer
	{
		public:
			LEOPPHAPI explicit Renderer(Window& window);

			LEOPPHAPI void render() const;
			LEOPPHAPI void set_sync_interval(UINT interval);

		private:
			Microsoft::WRL::ComPtr<ID3D11Device> mD3dDevice;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> mD3dDeviceContext;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
			#ifndef NDEBUG
			Microsoft::WRL::ComPtr<ID3D11Debug> mD3dDebug;
			#endif
			UINT mSyncInterval{0};

			UINT static constexpr SWAP_CHAIN_FLAGS{DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING};
	};
}
