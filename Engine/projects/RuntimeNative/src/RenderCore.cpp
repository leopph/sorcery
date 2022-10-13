#include "RenderCore.hpp"

#include <DirectXMath.h>

#include "Cube.hpp"
#include "Camera.hpp"

#ifdef NDEBUG
#include "CubeVertexShader.h"
#include "CubePixelShader.h"
#else
#include "CubeVertexShaderDebug.h"
#include "CubePixelShaderDebug.h"
#endif

#include <cassert>
#include <functional>
#include <utility>

using Microsoft::WRL::ComPtr;


namespace leopph
{
	UINT const RenderCore::sInstanceBufferElementSize{ sizeof(DirectX::XMFLOAT4X4) };
	UINT const RenderCore::sVertexBufferSlot{ 0 };
	UINT const RenderCore::sInstanceBufferSlot{ 1 };
	UINT const RenderCore::sSwapChainFlags{ DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING };
	UINT const RenderCore::sPresentFlags{ DXGI_PRESENT_ALLOW_TEARING };


	std::unique_ptr<RenderCore> RenderCore::Create(Window& window)
	{
		#ifdef NDEBUG
		UINT constexpr deviceCreationFlags = 0;
		#else
		UINT constexpr deviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;
		#endif

		D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		HRESULT hresult = D3D11CreateDevice(nullptr,
											D3D_DRIVER_TYPE_HARDWARE,
											nullptr,
											deviceCreationFlags,
											requestedFeatureLevels,
											1,
											D3D11_SDK_VERSION,
											device.GetAddressOf(),
											nullptr,
											context.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create D3D device.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		hresult = device.As(&d3dDebug);

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to get ID3D11Debug interface.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		hresult = d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue);

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to get ID3D11InfoQueue interface.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
		#endif

		ComPtr<IDXGIDevice> dxgiDevice;
		hresult = device.As(&dxgiDevice);

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to query IDXGIDevice interface.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hresult = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to get IDXGIAdapter.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ComPtr<IDXGIFactory2> dxgiFactory2;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType),
										 reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to query IDXGIFactory2 interface.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		DXGI_SWAP_CHAIN_DESC1 constexpr swapChainDesc1
		{
			.Width = 0,
			.Height = 0,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc =
			{
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling = DXGI_SCALING_NONE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = sSwapChainFlags
		};

		ComPtr<IDXGISwapChain1> swapChain;
		hresult = dxgiFactory2->CreateSwapChainForHwnd(device.Get(),
													   window.get_hwnd(),
													   &swapChainDesc1,
													   nullptr,
													   nullptr,
													   swapChain.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create swapchain.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		dxgiFactory2->MakeWindowAssociation(window.get_hwnd(), DXGI_MWA_NO_WINDOW_CHANGES);

		ComPtr<ID3D11Texture2D> backBuf;
		hresult = swapChain->GetBuffer(0,
									   __uuidof(decltype(backBuf)::InterfaceType),
									   reinterpret_cast<void**>(backBuf.GetAddressOf()));

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to get backbuffer.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ComPtr<ID3D11RenderTargetView> backBufRtv;
		hresult = device->CreateRenderTargetView(backBuf.Get(),
												 nullptr,
												 backBufRtv.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create backbuffer RTV.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ComPtr<ID3D11VertexShader> cubeVertShader;
		hresult = device->CreateVertexShader(gCubeVertShadBytes,
											 ARRAYSIZE(gCubeVertShadBytes),
											 nullptr,
											 cubeVertShader.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube vertex shader.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->VSSetShader(cubeVertShader.Get(), nullptr, 0);

		ComPtr<ID3D11PixelShader> cubePixShader;
		hresult = device->CreatePixelShader(gCubePixShadBytes,
											ARRAYSIZE(gCubePixShadBytes),
											nullptr,
											cubePixShader.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube pixel shader.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->PSSetShader(cubePixShader.Get(), nullptr, 0);

		D3D11_INPUT_ELEMENT_DESC constexpr inputElementDescs[]
		{
			{
				.SemanticName = "VERTEXPOS",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			},
			{
				.SemanticName = "MODELMATRIX",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			},
			{
				.SemanticName = "MODELMATRIX",
				.SemanticIndex = 1,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			},
			{
				.SemanticName = "MODELMATRIX",
				.SemanticIndex = 2,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			},
			{
				.SemanticName = "MODELMATRIX",
				.SemanticIndex = 3,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			}
		};

		ComPtr<ID3D11InputLayout> inputLayout;
		hresult = device->CreateInputLayout(inputElementDescs,
											ARRAYSIZE(inputElementDescs),
											gCubeVertShadBytes,
											ARRAYSIZE(gCubeVertShadBytes),
											inputLayout.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube input layout.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->IASetInputLayout(inputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		float constexpr cubeVertexData[]
		{
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
		};

		UINT constexpr vertexStride = 3 * sizeof(float);
		UINT constexpr vertexOffset = 0;

		D3D11_BUFFER_DESC constexpr cubeVertexBufferDesc
		{
			.ByteWidth = sizeof cubeVertexData,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA const cubeVertexSubresourceData
		{
			.pSysMem = cubeVertexData,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		ComPtr<ID3D11Buffer> vertexBuffer;
		hresult = device->CreateBuffer(&cubeVertexBufferDesc,
									   &cubeVertexSubresourceData,
									   vertexBuffer.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube vertex buffer.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->IASetVertexBuffers(sVertexBufferSlot,
									1,
									vertexBuffer.GetAddressOf(),
									&vertexStride,
									&vertexOffset);

		UINT const instanceBufferElementCapacity{ 1 };

		D3D11_BUFFER_DESC const desc
		{
			.ByteWidth = instanceBufferElementCapacity * sInstanceBufferElementSize,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		ComPtr<ID3D11Buffer> instanceBuffer;
		hresult = device->CreateBuffer(&desc, nullptr, instanceBuffer.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube instance buffer.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		UINT constexpr instanceBufferOffset{ 0 };

		context->IASetVertexBuffers(sInstanceBufferSlot,
									1,
									instanceBuffer.GetAddressOf(),
									&sInstanceBufferElementSize,
									&instanceBufferOffset);

		unsigned constexpr indexData[]
		{
			// Top face
			0, 1, 2,
			2, 3, 0,
			// Bottom face
			7, 6, 5,
			5, 4, 7,
			// Front face
			2, 6, 7,
			7, 3, 2,
			// Back face
			0, 4, 5,
			5, 1, 0,
			// Right face
			0, 3, 7,
			7, 4, 0,
			// Left face
			2, 1, 5,
			5, 6, 2
		};

		UINT const indexCount{ ARRAYSIZE(indexData) };

		D3D11_BUFFER_DESC constexpr indexBufferDesc
		{
			.ByteWidth = sizeof indexData,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA const indexSubresourceData
		{
			.pSysMem = indexData,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		ComPtr<ID3D11Buffer> indexBuffer;
		hresult = device->CreateBuffer(&indexBufferDesc,
									   &indexSubresourceData,
									   indexBuffer.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube index buffer.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		D3D11_BUFFER_DESC constexpr cbufferDesc
		{
			.ByteWidth = 16 * sizeof(float),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		ComPtr<ID3D11Buffer> cbuffer;
		hresult = device->CreateBuffer(&cbufferDesc,
									   nullptr,
									   cbuffer.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create cube constant buffer.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->VSSetConstantBuffers(0, 1, cbuffer.GetAddressOf());

		D3D11_RASTERIZER_DESC constexpr rasterizerDesc
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK,
			.FrontCounterClockwise = TRUE,
			.DepthBias = 0,
			.DepthBiasClamp = 0,
			.SlopeScaledDepthBias = 0,
			.DepthClipEnable = TRUE,
			.ScissorEnable = FALSE,
			.MultisampleEnable = FALSE,
			.AntialiasedLineEnable = FALSE
		};

		ComPtr<ID3D11RasterizerState> rasterizerState;
		hresult = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

		if (FAILED(hresult))
		{
			MessageBoxW(window.get_hwnd(), L"Failed to create rasterizer state.", L"Error", MB_ICONERROR);
			return nullptr;
		}

		context->RSSetState(rasterizerState.Get());

		Extent2D const renderRes{ window.get_current_client_area_size() };

		std::unique_ptr<RenderCore> ret{ new RenderCore{std::move(device),
														std::move(context),
														std::move(swapChain),
														std::move(backBufRtv),
														std::move(cubeVertShader),
														std::move(cubePixShader),
														renderRes,
														static_cast<f32>(renderRes.width) / static_cast<f32>(renderRes.height),
														std::move(instanceBuffer),
														instanceBufferElementCapacity,
														std::move(cbuffer),
														indexCount} };

		window.OnSizeEvent.add_handler(ret.get(), &on_window_resize);

		return ret;
	}


	RenderCore::RenderCore(Microsoft::WRL::ComPtr<ID3D11Device> device,
						   Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
						   Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain,
						   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufRtv,
						   Microsoft::WRL::ComPtr<ID3D11VertexShader> cubeVertShader,
						   Microsoft::WRL::ComPtr<ID3D11PixelShader> cubePixShader,
						   Extent2D const renderRes,
						   f32 const renderAspectRatio,
						   Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer,
						   UINT const instanceBufferElementCapacity,
						   Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer,
						   UINT const indexCount) :
		mDevice{ std::move(device) },
		mContext{ std::move(context) },
		mSwapChain{ std::move(swapChain) },
		mBackBufRtv{ std::move(backBufRtv) },
		mCubeVertShader{ std::move(cubeVertShader) },
		mCubePixShader{ std::move(cubePixShader) },
		mRenderRes{ renderRes },
		mRenderAspectRatio{ renderAspectRatio },
		mInstanceBuffer{ std::move(instanceBuffer) },
		mInstanceBufferElementCapacity{ instanceBufferElementCapacity },
		mCbuffer{ std::move(cbuffer) },
		mIndexCount{ indexCount }
	{}


	bool RenderCore::render()
	{
		if (cubePositions.empty())
		{
			mSwapChain->Present(0, sPresentFlags);
			return true;
		}

		if (cubePositions.size() > mInstanceBufferElementCapacity)
		{
			mInstanceBufferElementCapacity = static_cast<UINT>(cubePositions.size());

			D3D11_BUFFER_DESC const desc
			{
				.ByteWidth = mInstanceBufferElementCapacity * sInstanceBufferElementSize,
				.Usage = D3D11_USAGE_DYNAMIC,
				.BindFlags = D3D11_BIND_VERTEX_BUFFER,
				.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
				.MiscFlags = 0,
				.StructureByteStride = 0
			};

			HRESULT hresult = mDevice->CreateBuffer(&desc,
													nullptr,
													mInstanceBuffer.ReleaseAndGetAddressOf());
			if (FAILED(hresult))
			{
				MessageBoxW(nullptr, L"Failed to resize cube instance buffer.", L"Error", MB_ICONERROR);
				return false;
			}

			UINT constexpr instanceBufferOffset{ 0 };

			mContext->IASetVertexBuffers(sInstanceBufferSlot,
										 1,
										 mInstanceBuffer.GetAddressOf(),
										 &sInstanceBufferElementSize,
										 &instanceBufferOffset);
		}


		D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
		mContext->Map(mInstanceBuffer.Get(),
					  0,
					  D3D11_MAP_WRITE_DISCARD,
					  0,
					  &mappedInstanceBuffer);

		DirectX::XMFLOAT4X4* mappedInstanceBufferData{ static_cast<DirectX::XMFLOAT4X4*>(mappedInstanceBuffer.pData) };

		for (int i = 0; i < cubePositions.size(); i++)
		{
			DirectX::XMMATRIX const modelMat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&cubePositions[i])));
			DirectX::XMStoreFloat4x4(mappedInstanceBufferData + i, modelMat);

			i++;
		}

		mContext->Unmap(mInstanceBuffer.Get(), 0);

		DirectX::XMFLOAT3 const camPos{ leopph::camPos.get_data() };
		DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), { 0, 0, 1 }, { 0, 1, 0 });
		DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f / mRenderAspectRatio), mRenderAspectRatio, 0.3f, 100.f);

		D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
		mContext->Map(mCbuffer.Get(),
					  0,
					  D3D11_MAP_WRITE_DISCARD,
					  0,
					  &mappedCbuffer);

		DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewMat * projMat);
		mContext->Unmap(mCbuffer.Get(), 0);

		D3D11_VIEWPORT const viewPort
		{
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width = static_cast<FLOAT>(mRenderRes.width),
			.Height = static_cast<FLOAT>(mRenderRes.height),
			.MinDepth = 0,
			.MaxDepth = 1
		};
		mContext->RSSetViewports(1, &viewPort);

		FLOAT clearColor[]{ 0.5f, 0, 1, 1 };
		mContext->ClearRenderTargetView(mBackBufRtv.Get(), clearColor);
		mContext->OMSetRenderTargets(1, mBackBufRtv.GetAddressOf(), nullptr);

		mContext->DrawIndexedInstanced(mIndexCount,
									   static_cast<UINT>(cubePositions.size()),
									   0,
									   0,
									   0);

		mSwapChain->Present(0, sPresentFlags);
		return true;
	}


	void RenderCore::on_window_resize(RenderCore* const self, Extent2D const size)
	{
		if (size.width == 0 || size.height == 0)
		{
			return;
		}

		self->mBackBufRtv.Reset();
		self->mSwapChain->ResizeBuffers(0,
										0,
										0, DXGI_FORMAT_UNKNOWN,
										sSwapChainFlags);

		ComPtr<ID3D11Texture2D> backBuf;
		HRESULT hresult = self->mSwapChain->GetBuffer(0,
													  __uuidof(decltype(backBuf)::InterfaceType),
													  reinterpret_cast<void**>(backBuf.GetAddressOf()));
		assert(SUCCEEDED(hresult));

		hresult = self->mDevice->CreateRenderTargetView(backBuf.Get(),
														nullptr,
														self->mBackBufRtv.GetAddressOf());
		assert(SUCCEEDED(hresult));

		self->mRenderRes = size;
		self->mRenderAspectRatio = static_cast<f32>(size.width) / static_cast<f32>(size.height);
	}
}