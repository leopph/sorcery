#include "Mesh.hpp"

#include "Systems.hpp"
#include "Util.hpp"
#include "Serialization.hpp"
#include "Compress.hpp"

#include <utility>
#include <format>

namespace leopph {
auto Mesh::UploadToGPU() -> void {
	D3D11_BUFFER_DESC const posDesc{
		.ByteWidth = clamp_cast<UINT>(mData.positions.size() * sizeof(Vector3)),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const posBufData{
		.pSysMem = mData.positions.data()
	};

	if (FAILED(gRenderer.GetDevice()->CreateBuffer(&posDesc, &posBufData, mPosBuf.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh position buffer." };
	}

	D3D11_BUFFER_DESC const normDesc{
		.ByteWidth = clamp_cast<UINT>(mData.normals.size() * sizeof(Vector3)),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const normBufData{
		.pSysMem = mData.normals.data()
	};

	if (FAILED(gRenderer.GetDevice()->CreateBuffer(&normDesc, &normBufData, mNormBuf.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh normal buffer." };
	}

	D3D11_BUFFER_DESC const uvDesc{
		.ByteWidth = clamp_cast<UINT>(mData.uvs.size() * sizeof(Vector2)),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const uvBufData{
		.pSysMem = mData.uvs.data()
	};

	if (FAILED(gRenderer.GetDevice()->CreateBuffer(&uvDesc, &uvBufData, mUvBuf.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh uv buffer." };
	}

	D3D11_BUFFER_DESC const indDesc{
		.ByteWidth = clamp_cast<UINT>(mData.indices.size() * sizeof(u32)),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const indBufData{
		.pSysMem = mData.indices.data()
	};

	if (FAILED(gRenderer.GetDevice()->CreateBuffer(&indDesc, &indBufData, mIndBuf.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh index buffer." };
	}
}

auto Mesh::CalculateBounds() -> void {
	mBounds = {};

	for (auto const& position : mData.positions) {
		mBounds.min = Vector3{ std::min(mBounds.min[0], position[0]), std::min(mBounds.min[1], position[1]), std::min(mBounds.min[2], position[2]) };
		mBounds.max = Vector3{ std::max(mBounds.max[0], position[0]), std::max(mBounds.max[1], position[1]), std::max(mBounds.max[2], position[2]) };
	}
}

Mesh::Mesh(Data data) :
	mTempData{ std::move(data) } {
	ValidateAndUpdate();
}

auto Mesh::GetPositions() const noexcept -> std::span<Vector3 const> {
	return mData.positions;
}

auto Mesh::SetPositions(std::vector<Vector3> positions) noexcept -> void {
	mTempData.positions = std::move(positions);
}

auto Mesh::GetNormals() const noexcept -> std::span<Vector3 const> {
	return mData.normals;
}

auto Mesh::SetNormals(std::vector<Vector3> normals) noexcept -> void {
	mTempData.normals = std::move(normals);
}

auto Mesh::GetUVs() const noexcept -> std::span<Vector2 const> {
	return mData.uvs;
}

auto Mesh::SetUVs(std::vector<Vector2> uvs) noexcept -> void {
	mTempData.uvs = std::move(uvs);
}

auto Mesh::GetIndices() const noexcept -> std::span<u32 const> {
	return mData.indices;
}

auto Mesh::SetIndices(std::vector<u32> indices) noexcept -> void {
	mTempData.indices = std::move(indices);
}

auto Mesh::GetSubMeshes() const noexcept -> std::span<SubMeshData const> {
	return mData.subMeshes;
}

auto Mesh::SetSubMeshes(std::vector<SubMeshData> subMeshes) noexcept -> void {
	mTempData.subMeshes = std::move(subMeshes);
}

auto Mesh::GetBounds() const noexcept -> AABB const& {
	return mBounds;
}

auto Mesh::ValidateAndUpdate() -> void {
	auto constexpr errFmt{ "Failed to validate mesh {} (\"{}\"). {}." };

	if (mTempData.positions.size() != mTempData.normals.size() ||
	    mTempData.normals.size() != mTempData.uvs.size() ||
	    mTempData.positions.empty() || mTempData.indices.empty()) {
		throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "Inconsistent number of positions, normals, and UVs.") };
	}

	for (auto const& [baseVertex, firstIndex, indexCount] : mTempData.subMeshes) {
		if (baseVertex >= mTempData.positions.size()) {
			throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains a base vertex greater than the number of vertices.") };
		}

		if (firstIndex >= mTempData.indices.size()) {
			throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains a first index greater than the number of indices.") };
		}

		if (firstIndex + indexCount > mTempData.indices.size()) {
			throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains an out of bounds index region.") };
		}

		for (int i = firstIndex; i < firstIndex + indexCount; i++) {
			if (mTempData.indices[i] + baseVertex > mTempData.positions.size()) {
				throw std::runtime_error{ std::format(errFmt, GetGuid().ToString(), GetName(), "A submesh contains an out of bounds vertex index.") };
			}
		}
	}

	mData.positions = std::move(mTempData.positions);
	mData.normals = std::move(mTempData.normals);
	mData.uvs = std::move(mTempData.uvs);
	mData.indices = std::move(mTempData.indices);
	mData.subMeshes = std::move(mTempData.subMeshes);

	CalculateBounds();
	UploadToGPU();
}

auto Mesh::GetSerializationType() const -> Type {
	return Type::Mesh;
}

auto Mesh::GetPositionBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
	return mPosBuf;
}

auto Mesh::GetNormalBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
	return mNormBuf;
}

auto Mesh::GetUVBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
	return mUvBuf;
}

auto Mesh::GetIndexBuffer() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Buffer> {
	return mIndBuf;
}

void Mesh::SetData(Data data) noexcept {
	mTempData = std::move(data);
}
}
