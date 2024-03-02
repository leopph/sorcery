#pragma once

#include "../graphics_platform.hpp"

#include <cassert>
#include <span>
#include <stdexcept>


namespace sorcery {
template<typename T>
class StructuredBuffer {
  static_assert(sizeof(T) % 16 == 0, "StructuredBuffer contained type must have a size divisible by 16.");

  Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mBuffer;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  T* mMappedPtr{nullptr};
  int mCapacity{1};
  int mSize{0};

  auto RecreateBuffer() -> void;
  auto RecreateSrv() -> void;

public:
  explicit StructuredBuffer(Microsoft::WRL::ComPtr<ID3D11Device> device);

  [[nodiscard]] auto GetSrv() const noexcept -> ID3D11ShaderResourceView*;

  // The buffer must be in an unmapped state before the call!
  auto Resize(int newSize) -> void;

  [[nodiscard]] auto Map(ObserverPtr<ID3D11DeviceContext> ctx) -> std::span<T>;
  auto Unmap(ObserverPtr<ID3D11DeviceContext> ctx) -> void;
};


template<typename T>
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  D3D11_BUFFER_DESC const bufDesc{.ByteWidth = mCapacity * sizeof(T), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_SHADER_RESOURCE, .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, .StructureByteStride = sizeof(T)};

  if (FAILED(mDevice->CreateBuffer(&bufDesc, nullptr, mBuffer.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to recreate StructuredBuffer buffer."};
  }
}


template<typename T>
auto StructuredBuffer<T>::RecreateSrv() -> void {
  if (mSize == 0) {
    mSrv.Reset();
  } else {
    D3D11_SHADER_RESOURCE_VIEW_DESC const srvDesc{.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D11_SRV_DIMENSION_BUFFER, .Buffer = {.FirstElement = 0, .NumElements = static_cast<UINT>(mSize)}};

    if (FAILED(mDevice->CreateShaderResourceView(mBuffer.Get(), &srvDesc, mSrv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to recreate StructuredBuffer SRV."};
    }
  }
}


template<typename T>
StructuredBuffer<T>::StructuredBuffer(Microsoft::WRL::ComPtr<ID3D11Device> device):
  mDevice{std::move(device)} {
  RecreateBuffer();
}


template<typename T>
auto StructuredBuffer<T>::GetSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mSrv.Get();
}


template<typename T>
auto StructuredBuffer<T>::Resize(int const newSize) -> void {
  assert(!mMappedPtr);

  auto newCapacity = mCapacity;

  while (newCapacity < newSize) {
    newCapacity *= 2;
  }

  if (newCapacity != mCapacity) {
    mCapacity = newCapacity;
    mSize = newSize;
    RecreateBuffer();
    RecreateSrv();
  } else if (mSize != newSize) {
    mSize = newSize;
    RecreateSrv();
  }
}


template<typename T>
auto StructuredBuffer<T>::Map(ObserverPtr<ID3D11DeviceContext> const ctx) -> std::span<T> {
  if (mMappedPtr) {
    return {mMappedPtr, static_cast<std::size_t>(mSize)};
  }

  D3D11_MAPPED_SUBRESOURCE mappedSubresource;
  if (FAILED(ctx->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource))) {
    throw std::runtime_error{"Failed to map Structured Buffer."};
  }

  mMappedPtr = static_cast<T*>(mappedSubresource.pData);
  return {static_cast<T*>(mMappedPtr), static_cast<std::size_t>(mSize)};
}


template<typename T>
auto StructuredBuffer<T>::Unmap(ObserverPtr<ID3D11DeviceContext> const ctx) -> void {
  if (mMappedPtr) {
    ctx->Unmap(mBuffer.Get(), 0);
    mMappedPtr = nullptr;
  }
}
}
