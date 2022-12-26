#include "ModelImport.hpp"

#include "BinarySerializer.hpp"
#include "Compress.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


#include <fstream>
#include <map>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <utility>


namespace leopph {
	namespace {
		[[nodiscard]] auto ConvertMatrix(aiMatrix4x4 const& aiMat) noexcept -> Matrix4 {
			return Matrix4{
				aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
				aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
				aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
				aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
			};
		}

		[[nodiscard]] auto ConvertVector(aiVector3D const& aiVec) noexcept -> Vector3 {
			return Vector3{ aiVec.x, aiVec.y, aiVec.z };
		}

		[[nodiscard]] auto ConvertColor(aiColor3D const& aiCol) noexcept -> Color {
			return Color{
				static_cast<u8>(aiCol.r * 255),
				static_cast<u8>(aiCol.g * 255),
				static_cast<u8>(aiCol.b * 255),
				255
			};
		}

		[[nodiscard]] auto LoadTexture(aiString const& texPath, std::filesystem::path const& rootPath, aiScene const& scene) noexcept -> Image {
			// Texture is embedded
			if (auto const* const embeddedTexture = scene.GetEmbeddedTexture(texPath.C_Str()); embeddedTexture) {
				// Texture is not compressed
				if (embeddedTexture->mHeight) {
					auto const numBytes = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
					auto imgData = std::make_unique_for_overwrite<u8[]>(numBytes);
					std::memcpy(imgData.get(), embeddedTexture->pcData, numBytes);

					return Image{ embeddedTexture->mWidth, embeddedTexture->mHeight, 4, std::move(imgData) };
				}

				// Texture is compressed
				// TODO implement support for compressed embedded textures.
				//Logger::get_instance().error("Skipped loading embedded texture, becuase it was compressed. Compressed embedded textures are not yet supported.");
				return Image{};
			}

			// Texture is in a separate file
			return Image{ rootPath / texPath.C_Str(), ImageOrientation::FlipVertical };
		};
	};


	auto ImportModelAsset(std::filesystem::path const& srcFile) -> ModelImportData {
		Assimp::Importer importer;

		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

		auto const scene{ importer.ReadFile(srcFile.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent) };

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			throw std::runtime_error{ std::format("Failed to import model at {}: {}.", srcFile.string(), importer.GetErrorString()) };
		}

		auto const& srcDir = srcFile.parent_path();

		ModelImportData ret;
		
		/*std::unordered_map<aiString, std::size_t> texPathToInd;

		auto const LoadOrGetTextureIndexFromCache = [&ret, &texPathToInd, &srcDir, scene](aiString const& texPath) -> unsigned long long {
			if (auto const it = texPathToInd.find(texPath); it != std::end(texPathToInd)) {
				return it->second;
			}

			ret.textures.emplace_back(LoadTexture(texPath, srcDir, *scene));
			ret.textures.back().image.set_encoding(ColorEncoding::sRGB);
			texPathToInd[texPath] = ret.textures.size() - 1;
			return ret.textures.size() - 1;
		};*/
		
		ret.materials.reserve(scene->mNumMaterials);

		for (unsigned i{ 0 }; i < scene->mNumMaterials; i++) {
			auto const* const aiMat = scene->mMaterials[i];
			auto& mat{ ret.materials.emplace_back() };

			if (aiColor3D baseColor; aiMat->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS) {
				mat.albedo = ConvertColor(baseColor);
			}

			if (f32 metallic; aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS) {
				mat.metallic = metallic;
			}

			if (f32 roughness; aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS) {
				mat.roughness = roughness;
			}

			/*if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), texPath) == aiReturn_SUCCESS) {
				mat.albedoMapIndex = LoadOrGetTextureIndexFromCache(texPath);
			}

			if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_METALNESS, 0), texPath) == aiReturn_SUCCESS) {
				mat.metallicMapIndex = LoadOrGetTextureIndexFromCache(texPath);
			}

			if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE_ROUGHNESS, 0), texPath) == aiReturn_SUCCESS) {
				mat.roughnessMapIndex = LoadOrGetTextureIndexFromCache(texPath);
			}*/
		}

		// Extract geometry data from the assimp structure, group it by the used materials, and calculate AABB for the meshes

		struct SubMeshData {
			std::vector<Vertex> vertices{};
			std::vector<u32> indices{};
		};

		// Maps a material index to a list of meshes
		std::map<std::size_t, std::vector<SubMeshData>> subMeshDataByMaterial;


		std::queue<std::pair<aiNode const*, Matrix4>> queue;
		queue.emplace(scene->mRootNode, ConvertMatrix(scene->mRootNode->mTransformation).transpose() * Matrix4{ 1, 1, -1, 1 });

		while (!queue.empty()) {
			auto& [node, trafo] = queue.front();

			for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
				// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
				// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
				if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
					if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
						//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
						// TODO transform to triangle fans
					}

					auto& [vertices, indices]{ subMeshDataByMaterial[mesh->mMaterialIndex].emplace_back() };

					// Process the vertices
					for (unsigned j = 0; j < mesh->mNumVertices; j++) {
						vertices.emplace_back(
							Vector3{ Vector4{ ConvertVector(mesh->mVertices[j]), 1 } * trafo },
							Vector3{ Vector4{ ConvertVector(mesh->mNormals[j]), 0 } * trafo }.normalize(),
							[mesh, j] {
								for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
									if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
										return Vector2{ ConvertVector(mesh->mTextureCoords[k][j]) };
									}
								}
								return Vector2{};
							}()
						);
					}

					// Process the indices
					for (unsigned j = 0; j < mesh->mNumFaces; j++) {
						for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++) {
							indices.push_back(mesh->mFaces[j].mIndices[k]);
						}
					}
				}
				else {
					std::string primitiveType;

					if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT) {
						primitiveType += " [points]";
					}

					if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) {
						primitiveType += " [lines]";
					}

					if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) {
						primitiveType += " [N>3 polygons]";
					}

					// TODO Implement non-triangle rendering support
					//Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType));
				}
			}

			for (std::size_t i = 0; i < node->mNumChildren; ++i) {
				queue.emplace(node->mChildren[i], ConvertMatrix(node->mChildren[i]->mTransformation).transpose() * trafo);
			}

			queue.pop();
		}

		// Collapse meshes over materials and calculate their AABBs

		for(auto& subMeshes : subMeshDataByMaterial | std::views::values) {
			auto& mesh{ ret.meshes.emplace_back() };

			for (auto& [vertices, indices] : subMeshes) {
				for (auto& index : indices) {
					index += clamp_cast<u32>(mesh.vertices.size());
				}

				mesh.vertices.insert(std::end(mesh.vertices), std::begin(vertices), std::end(vertices));
				mesh.indices.insert(std::end(mesh.indices), std::begin(indices), std::end(indices));

				mesh.bounds = CalculateBounds(mesh.vertices);
			}
		}

		return ret;
	}


	auto CalculateBounds(std::span<Vertex const> const vertices) noexcept -> AABB {
		AABB bounds{};

		for (auto const& [position, normal, uv] : vertices) {
			bounds.min = Vector3{ std::min(bounds.min[0], position[0]), std::min(bounds.min[1], position[1]), std::min(bounds.min[2], position[2]) };
			bounds.max = Vector3{ std::max(bounds.max[0], position[0]), std::max(bounds.max[1], position[1]), std::max(bounds.max[2], position[2]) };
		}

		return bounds;
	}


	/*auto GenerateModelLeopphAsset(ModelData const& modelData, std::vector<u8>& out, std::endian const endianness) -> std::vector<u8>& {
		if (endianness != std::endian::little && endianness != std::endian::big) {
			auto const msg = "Error while exporting. Mixed endian architectures are currently not supported.";
			//internal::Logger::Instance().Critical(msg);
			throw std::logic_error{ msg };
		}

		// signature bytes
		out.push_back('x');
		out.push_back('d');
		out.push_back('6');
		out.push_back('9');

		// the version number. its MSB is used to indicate if the file is little endian
		out.push_back(0x01 | (endianness == std::endian::big ? 0 : 0x80));

		std::vector<u8> toCompress;
		BinarySerializer<ModelData>::Serialize(modelData, out, endianness);

		switch (std::vector<u8> compressed; Compress(toCompress, compressed)) {
			case CompressionError::None: {
				BinarySerializer<u64>::Serialize(toCompress.size(), out, endianness);
				std::ranges::copy(compressed, std::back_inserter(out));
				break;
			}

			case CompressionError::Inconsistency: {
				throw std::runtime_error{ "Failed to compress asset file contents due to an inconsitency error." };
			}

			case CompressionError::Unknown: {
				throw std::runtime_error{ "Failed to compress asset file contents due to an unknown error." };
			}
		}

		return out;
	}

	auto LoadLeopphModelAsset(std::filesystem::path const& src) -> ModelData {
		constexpr u64 HEADER_SZ = 13;

		std::ifstream in{ src, std::ios::in | std::ios::binary };

		// disable whitespace skipping
		in.unsetf(std::ios::skipws);

		// failed to open file
		if (!in.is_open()) {
			throw std::runtime_error{ std::format("Failed to parse leopphasset file at {}, because the file does not exist.", src.string()) };
		}

		std::vector<u8> buffer(HEADER_SZ);

		// read header
		in.read(reinterpret_cast<char*>(buffer.data()), HEADER_SZ);

		// failed to read header
		if (in.eof() || in.fail() ||
			buffer[0] != 'x' ||
			buffer[1] != 'd' ||
			buffer[2] != '6' ||
			buffer[3] != '9') {
			throw std::runtime_error{ std::format("Failed to parse leopphasset file at {}, because the file is corrupted or invalid.", src.string()) };
		}

		// parse endianness
		auto const endianness = buffer[4] & 0x80 ? std::endian::little : std::endian::big;

		// parse content size
		auto const contentSize = BinarySerializer<u64>::Deserialize(std::span{ buffer }.subspan<5, 8>(), endianness);

		// get the size of the compressed contents
		in.seekg(0, std::ios_base::end);
		auto const comprSz = static_cast<u64>(in.tellg()) - HEADER_SZ;

		// read rest of the file
		buffer.resize(comprSz);
		in.seekg(HEADER_SZ, std::ios_base::beg);
		in.read(reinterpret_cast<char*>(buffer.data()), comprSz);

		std::vector<u8> uncompressed;

		// uncompress data
		if (Uncompress({ std::begin(buffer), std::end(buffer) }, contentSize, uncompressed) != CompressionError::None) {
			throw std::runtime_error{ std::format("Failed to parse leopphasset file at {}, because the file contents could not be uncompressed.", src.string()) };
		}

		return BinarySerializer<ModelData>::Deserialize(std::span{ uncompressed }, endianness);
	}*/
}
