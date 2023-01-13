#include "Mesh.hpp"

#include "Systems.hpp"
#include "Util.hpp"
#include "BinarySerializer.hpp"

#include <utility>
#include <format>

namespace leopph {
	auto Mesh::UploadToGPU() -> void {
		if (mData.positions.size() != mData.normals.size() ||
			mData.normals.size() != mData.uvs.size() ||
			mData.positions.empty() ||
			mData.indices.empty()) {
			throw std::runtime_error{ "Inconsistency detected in mesh data while uploading Mesh to GPU." };
		}

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

		if (FAILED(gRenderer.GetDevice()->CreateBuffer(&posDesc, &posBufData, mPosBuf.GetAddressOf()))) {
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

		if (FAILED(gRenderer.GetDevice()->CreateBuffer(&normDesc, &normBufData, mNormBuf.GetAddressOf()))) {
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

		if (FAILED(gRenderer.GetDevice()->CreateBuffer(&uvDesc, &uvBufData, mUvBuf.GetAddressOf()))) {
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

		if (FAILED(gRenderer.GetDevice()->CreateBuffer(&indDesc, &indBufData, mIndBuf.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create mesh index buffer." };
		}
	}

	Mesh::Mesh(Data data) :
		mData{ std::move(data) } {
		UploadToGPU();
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

	auto Mesh::ValidateAndUpdate() -> void {
		auto const errMsg{ std::format("Failed to validate mesh {} (\"{}\").", GetGuid().ToString(), GetName()) };

		if (mTempData.positions.size() != mTempData.normals.size() ||
			mTempData.normals.size() != mTempData.uvs.size() ||
			mTempData.positions.empty() || mTempData.indices.empty()) {
			throw std::runtime_error{ errMsg };
		}

		for (auto const ind : mTempData.indices) {
			if (ind >= mTempData.positions.size()) {
				throw std::runtime_error{ errMsg };
			}
		}

		mData.positions = std::move(mTempData.positions);
		mData.normals = std::move(mTempData.normals);
		mData.uvs = std::move(mTempData.uvs);
		mData.indices = std::move(mTempData.indices);

		UploadToGPU();
	}

	auto Mesh::GetSerializationType() const -> Type {
		return Type::Mesh;
	}


	auto Mesh::SerializeBinary(std::vector<u8>& out) const -> void {
		Object::SerializeBinary(out);
		BinarySerializer<u64>::Serialize(mData.positions.size(), out, std::endian::native);
		BinarySerializer<u64>::Serialize(mData.indices.size(), out, std::endian::native);

		for (auto const& pos : mData.positions) {
			BinarySerializer<Vector3>::Serialize(pos, out, std::endian::native);
		}

		for (auto const& norm : mData.normals) {
			BinarySerializer<Vector3>::Serialize(norm, out, std::endian::native);
		}

		for (auto const& uv : mData.uvs) {
			BinarySerializer<Vector2>::Serialize(uv, out, std::endian::native);
		}

		for (auto const ind : mData.indices) {
			BinarySerializer<u32>::Serialize(ind, out, std::endian::native);
		}
	}

	auto Mesh::DeserializeBinary(std::span<u8 const> const bytes) -> BinaryDeserializationResult {
		auto ret{ Object::DeserializeBinary(bytes) };

		if (bytes.size() < 16) {
			throw std::runtime_error{ "Failed to deserialize Mesh, because span does not contain enough bytes to read size information." };
		}

		auto const numVerts{ BinarySerializer<u64>::Deserialize(bytes.first<8>(), std::endian::native) };
		auto const numInds{ BinarySerializer<u64>::Deserialize(bytes.subspan<8, 8>(), std::endian::native) };

		auto const numBytesConsumed{ 16 + numVerts * (2 * sizeof(Vector3) + sizeof(Vector2)) + numInds * sizeof(u32) };

		if (bytes.size() < numBytesConsumed) {
			throw std::runtime_error{ "Failed to deserialize Mesh, because span does not contain enough bytes to read data." };
		}

		mData.positions.clear();
		mData.positions.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			mData.positions.emplace_back(BinarySerializer<Vector3>::Deserialize(bytes.subspan(16 + i * sizeof(Vector3)).first<sizeof(Vector3)>(), std::endian::native));
		}

		mData.normals.clear();
		mData.normals.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			mData.normals.emplace_back(BinarySerializer<Vector3>::Deserialize(bytes.subspan(16 + numVerts * sizeof(Vector3) + i * sizeof(Vector3)).first<sizeof(Vector3)>(), std::endian::native));
		}

		mData.uvs.clear();
		mData.uvs.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			mData.uvs.emplace_back(BinarySerializer<Vector2>::Deserialize(bytes.subspan(16 + numVerts * 2 * sizeof(Vector3) + i * sizeof(Vector2)).first<sizeof(Vector2)>(), std::endian::native));
		}

		mData.indices.clear();
		mData.indices.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numInds; i++) {
			mData.indices.emplace_back(BinarySerializer<u32>::Deserialize(bytes.subspan(16 + numVerts * (2 * sizeof(Vector3) + sizeof(Vector2)) + i * sizeof(u32)).first<sizeof(u32)>(), std::endian::native));
		}

		UploadToGPU();

		ret.numBytesConsumed += numBytesConsumed;
		return ret;
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

}
