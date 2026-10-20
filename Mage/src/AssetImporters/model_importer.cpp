#include "model_importer.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <assimp/DefaultLogger.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/Logger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/entity_serialization.hpp"
#include "sorcery/mesh_utils.hpp"
#include "sorcery/observer_ptr.hpp"
#include "sorcery/Serialization.hpp"
#include "sorcery/util.hpp"
#include "sorcery/assets/Mesh.hpp"
#include "sorcery/assets/prefab.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/platform.hpp"
#include "sorcery/host/assets/material_import.hpp"
#include "sorcery/host/assets/texture_import.hpp"
#include "sorcery/scene/Entity.hpp"
#include "sorcery/scene/StaticMeshComponent.hpp"


RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::mage::ModelImporter::SubassetKind>("ModelImporterSubassetKind")(
    rttr::value("Material", sorcery::mage::ModelImporter::SubassetKind::kMaterial),
    rttr::value("Texture", sorcery::mage::ModelImporter::SubassetKind::kTexture)
  );

  rttr::registration::class_<sorcery::mage::ModelImporter::SubassetImportSettings>(
      "ModelImporterSubassetImportSettings")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Subasset Kind", &sorcery::mage::ModelImporter::SubassetImportSettings::kind)
    .property("Material Import Settings", &sorcery::mage::ModelImporter::SubassetImportSettings::material_settings)
    .property("Texture Import Settings", &sorcery::mage::ModelImporter::SubassetImportSettings::texture_settings);

  rttr::registration::class_<sorcery::mage::ModelImporter>{"Model Importer"}
    .REFLECT_REGISTER_ASSET_IMPORTER_CTOR
    .property("Subassets", &sorcery::mage::ModelImporter::subasset_import_settings_)
    .property("Mesh Import Settings", &sorcery::mage::ModelImporter::mesh_import_settings_)
    .property("Import materials", &sorcery::mage::ModelImporter::import_materials_)
    .property("Import textures", &sorcery::mage::ModelImporter::import_textures_);
}


namespace sorcery::mage {
namespace {
struct Node {
  std::string name;
  Matrix4 transform;
  Node* parent{nullptr};
  std::vector<std::unique_ptr<Node>> children;
  bool visited{false};
};


constexpr auto kBlendFallbackAlphaThreshold = 0.1f;


using SubassetImportSettings = ModelImporter::SubassetImportSettings;
using SubassetKind = ModelImporter::SubassetKind;


/******************************************************
 WE WANT TO IMPORT SUBASSETS IN THE FOLLOWING ORDER:
 1. Prefab
 2. Meshes
 3. Materials
 4. Textures
 *******************************************************/

// Maps subassets to their package indices and to their indices in the subasset import settings.
// The subasset import settings only hold materials and textures.
class SubassetLayout {
public:
  SubassetLayout(std::size_t const mesh_count, std::size_t const material_count) :
    mesh_count_{mesh_count},
    material_count_{material_count} {}


  [[nodiscard]]
  auto GetMeshIdx(std::size_t const mesh_idx) const -> std::size_t {
    assert(mesh_idx < mesh_count_);
    return 1uz /* prefab is first */ + mesh_idx;
  }


  [[nodiscard]]
  auto GetMaterialIdx(std::size_t const mtl_idx) const -> std::size_t {
    assert(mtl_idx < material_count_);
    return 1uz /* prefab */ + mesh_count_ + mtl_idx;
  }


  [[nodiscard]]
  auto GetTextureIdx(std::size_t const tex_idx) const -> std::size_t {
    return 1uz /* prefab */ + mesh_count_ + material_count_ + tex_idx;
  }


  [[nodiscard]] static
  auto GetMaterialSettingsIdx(std::size_t const mtl_idx) -> std::size_t {
    return mtl_idx;
  }


  [[nodiscard]]
  auto GetTextureSettingsIdx(std::size_t const tex_idx) const -> std::size_t {
    return material_count_ + tex_idx;
  }


  [[nodiscard]]
  auto GetMeshCount() const -> std::size_t {
    return mesh_count_;
  }


  [[nodiscard]]
  auto GetMaterialCount() const -> std::size_t {
    return material_count_;
  }

private:
  std::size_t mesh_count_;
  std::size_t material_count_;
};


struct TextureLoadInfo {
  aiTexture const* tex;
  aiTextureType type;

  [[nodiscard]]
  auto operator==(TextureLoadInfo const& other) const -> bool = default;
};


struct CollectedMaterials {
  std::vector<MaterialSlotInfo> slots;
  std::vector<MaterialAssetData> assets;
  std::vector<TextureLoadInfo> tex_deps;
};


struct MeshProcessingData {
  std::vector<Vector3> positions;
  std::vector<Vector3> normals;
  std::vector<Vector2> uvs;
  std::vector<Vector3> tangents;
  std::vector<unsigned> indices;
  std::vector<Vector4> bone_weights;
  std::vector<Vector<std::uint32_t, 4>> bone_indices;
  unsigned mtl_idx{};
};


struct BoneProcessingInfo {
  Matrix4 offset_matrix;
  std::string node_name;
};


[[nodiscard]]
auto Convert(aiVector3D const& ai_vec) noexcept -> Vector3 {
  return Vector3{ai_vec.x, ai_vec.y, ai_vec.z};
}


[[nodiscard]]
auto Convert(aiQuaternion const& ai_quat) noexcept -> Quaternion {
  return Quaternion{ai_quat.w, ai_quat.x, ai_quat.y, ai_quat.z};
}


[[nodiscard]]
auto Convert(aiMatrix4x4 const& ai_mat) noexcept -> Matrix4 {
  return Matrix4{
    ai_mat.a1, ai_mat.a2, ai_mat.a3, ai_mat.a4,
    ai_mat.b1, ai_mat.b2, ai_mat.b3, ai_mat.b4,
    ai_mat.c1, ai_mat.c2, ai_mat.c3, ai_mat.c4,
    ai_mat.d1, ai_mat.d2, ai_mat.d3, ai_mat.d4
  };
}


[[nodiscard]]
auto Convert(aiNode const* node, Node* const converted_parent) -> std::unique_ptr<Node> {
  auto converted_node{
    std::make_unique<Node>(node->mName.C_Str(), Convert(node->mTransformation).Transpose(), converted_parent)
  };

  for (unsigned i{0}; i < node->mNumChildren; i++) {
    converted_node->children.emplace_back(Convert(node->mChildren[i], converted_node.get()));
  }

  return converted_node;
}


[[nodiscard]]
auto FindInHierarchyByName(Node& root_node, std::string_view const name) -> Node* {
  if (root_node.name == name) {
    return &root_node;
  }

  for (auto const& child : root_node.children) {
    if (auto const found{FindInHierarchyByName(*child, name)}) {
      return found;
    }
  }

  return nullptr;
}


[[nodiscard]]
auto IsSrgb(aiTextureType const tex_type) -> bool {
  return
    tex_type == aiTextureType_BASE_COLOR ||
    tex_type == aiTextureType_DIFFUSE ||
    tex_type == aiTextureType_EMISSIVE ||
    tex_type == aiTextureType_EMISSION_COLOR;
}


// Returns nullptr on failure. The importer owns the scene.
[[nodiscard]]
auto LoadScene(std::filesystem::path const& src) -> std::unique_ptr<Assimp::Importer> {
  if (spdlog::get_level() == spdlog::level::trace) {
    Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT | aiDefaultLogStream_DEBUGGER);
  }

  auto importer = std::make_unique<Assimp::Importer>();

  // We don't need these scene objects
  importer->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  // We don't want to bother with non-triangle primitives
  importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  // Smoothing angle for smooth normal generation
  importer->SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
  // By default, assimp’s FBX importer keeps pivot transforms as extra nodes named …$AssimpFbx$_Translation, …_Rotation and so on.
  // The hierarchy would show them as clutter. This flag folds them into a single transform per node.
  importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  auto const scene{
    importer->ReadFile(std::string{ToUntypedSv(src.u8string())},
      aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_TransformUVCoords |
      aiProcess_RemoveComponent)
  };

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    DisplayError(std::format("Failed to import model at {}: {}.", ToUntypedSv(src.u8string()),
      importer->GetErrorString()));
    return nullptr;
  }

  return std::move(importer);
}


// Store the texture or return the index of the existing one if it is already stored.
auto TryEmplaceTexture(std::vector<TextureLoadInfo>& tex_infos, aiTexture const& tex,
                       aiTextureType const type) -> std::size_t {
  // If type is unknown, be only place it if we don't already have it.
  // Otherwise we just return the first one we find, since we don't care about the type.
  if (type == aiTextureType_UNKNOWN) {
    if (auto const it{std::ranges::find(tex_infos, &tex, &TextureLoadInfo::tex)}; it != tex_infos.end()) {
      return std::distance(tex_infos.begin(), it);
    }

    tex_infos.emplace_back(&tex, type);
    return tex_infos.size() - 1;
  }

  // If type is concrete, we only look for exact matches, and if that fails, then we store.
  if (auto const it{std::ranges::find(tex_infos, TextureLoadInfo{.tex = &tex, .type = type})};
    it != tex_infos.end()) {
    return std::distance(tex_infos.begin(), it);
  }

  tex_infos.emplace_back(&tex, type);
  return tex_infos.size() - 1;
}


// Texture map ids of the returned material hold raw tex_infos indices. They must be fixed up later.
[[nodiscard]]
auto ExtractMaterial(aiScene const& scene, aiMaterial const& mtl, bool const import_textures,
                     std::vector<TextureLoadInfo>& tex_infos) -> MaterialAssetData {
  MaterialAssetData mtl_data;

  if (aiColor4D base_color; mtl.Get(AI_MATKEY_BASE_COLOR, base_color) == aiReturn_SUCCESS) {
    mtl_data.base_color = Vector3{base_color.r, base_color.g, base_color.b};
    mtl_data.opacity = base_color.a;
  } else if (float opacity; mtl.Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS) {
    mtl_data.opacity = opacity;
  }

  if (float metallic; mtl.Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS) {
    mtl_data.metallic = metallic;
  }

  if (float roughness; mtl.Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS) {
    mtl_data.roughness = roughness;
  }

  if (aiColor3D emissive_color; mtl.Get(AI_MATKEY_COLOR_EMISSIVE, emissive_color) == aiReturn_SUCCESS) {
    mtl_data.emissive = Vector3{emissive_color.r, emissive_color.g, emissive_color.b};
  }

  if (float emissive_intensity; mtl.Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive_intensity) == aiReturn_SUCCESS) {
    mtl_data.emissive_intensity = emissive_intensity;
  }

  std::optional<MaterialBlendMode> explicit_blend_mode;
  auto mtl_alpha_threshold = 0.5f;

  if (aiString alpha_mode; mtl.Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode) == AI_SUCCESS) {
    if (std::string_view const mode{alpha_mode.C_Str()}; mode == "MASK") {
      explicit_blend_mode = MaterialBlendMode::kAlphaClip;
      mtl.Get(AI_MATKEY_GLTF_ALPHACUTOFF, mtl_alpha_threshold);
    } else if (mode == "BLEND") {
      // TODO add alpha blending support
      // Clip only near-invisible texels so semi-transparent surfaces render opaque instead of vanishing.
      spdlog::warn("Material \"{}\" uses alpha blending, which is not supported. Falling back to alpha clipping.",
        mtl.GetName().C_Str());
      explicit_blend_mode = MaterialBlendMode::kAlphaClip;
      mtl_alpha_threshold = kBlendFallbackAlphaThreshold;
    } else {
      explicit_blend_mode = MaterialBlendMode::kOpaque;
    }
  }

  auto has_opacity_override = false;
  auto base_color_uses_alpha = false;

  if (import_textures) {
    auto const find_embedded_tex = [&scene, &mtl]
    (aiTextureType const type, unsigned const idx) -> ObserverPtr<aiTexture const> {
      aiString tex_path;

      if (mtl.GetTexture(type, idx, &tex_path) != aiReturn_SUCCESS) {
        return nullptr;
      }

      return MakeObserver(scene.GetEmbeddedTexture(tex_path.C_Str()));
    };

    auto const register_tex = [&tex_infos]
    (aiTexture const& tex, aiTextureType const type, AssetId& id_to_set) {
      // We store the textures' array index now as their package index.
      // Later we will patch this when we're creating the final subasset order in the package.
      auto const arr_idx = TryEmplaceTexture(tex_infos, tex, type);
      id_to_set = AssetId{Guid::Invalid(), static_cast<int>(arr_idx)};
    };

    auto const discover_embedded_tex = [&]
    (aiTextureType const type, unsigned const idx, AssetId& id_to_set) {
      auto const tex = find_embedded_tex(type, idx);

      if (tex) {
        register_tex(*tex, type, id_to_set);
      }

      return tex;
    };

    auto base_color_tex_type = aiTextureType_BASE_COLOR;
    auto base_color_tex = discover_embedded_tex(aiTextureType_BASE_COLOR, 0, mtl_data.base_color_map);

    if (!base_color_tex) {
      base_color_tex = discover_embedded_tex(aiTextureType_DIFFUSE, 0, mtl_data.base_color_map);
      base_color_tex_type = aiTextureType_DIFFUSE;
    }

    discover_embedded_tex(aiTextureType_METALNESS, 0, mtl_data.metallic_map);
    discover_embedded_tex(aiTextureType_DIFFUSE_ROUGHNESS, 0, mtl_data.roughness_map);
    discover_embedded_tex(aiTextureType_AMBIENT_OCCLUSION, 0, mtl_data.ao_map);
    discover_embedded_tex(aiTextureType_EMISSIVE, 0, mtl_data.emissive_map) ||
      discover_embedded_tex(aiTextureType_EMISSION_COLOR, 0, mtl_data.emissive_map);
    discover_embedded_tex(aiTextureType_NORMALS, 0, mtl_data.normal_map);

    auto const opacity_tex = find_embedded_tex(aiTextureType_OPACITY, 0);

    // The opacity source may be the base color tex itself, in which case we don't want to double-import it
    auto const opacity_is_base_color_alpha = opacity_tex && opacity_tex == base_color_tex;
    has_opacity_override = opacity_tex && !opacity_is_base_color_alpha;

    if (has_opacity_override) {
      register_tex(*opacity_tex, aiTextureType_OPACITY, mtl_data.opacity_map);
      // We consider separate opacity maps grayscale
      mtl_data.opacity_channel = 0;
    }

    base_color_uses_alpha = opacity_is_base_color_alpha;

    if (int flags; base_color_tex &&
                   mtl.Get(AI_MATKEY_TEXFLAGS(base_color_tex_type, 0), flags) == aiReturn_SUCCESS) {
      base_color_uses_alpha = base_color_uses_alpha || flags & aiTextureFlags_UseAlpha;
    }
  }

  if (explicit_blend_mode) {
    mtl_data.blend_mode = *explicit_blend_mode;
  } else if (has_opacity_override || base_color_uses_alpha) {
    mtl_data.blend_mode = MaterialBlendMode::kAlphaClip;
  }

  mtl_data.alpha_threshold = mtl_alpha_threshold;

  if (!explicit_blend_mode && mtl_data.blend_mode == MaterialBlendMode::kOpaque &&
      mtl_data.opacity < 1.0f) {
    spdlog::info("Material \"{}\" has an opacity factor of {} but no opacity texture. Importing as opaque.",
      mtl.GetName().C_Str(), mtl_data.opacity);
  }

  return mtl_data;
}


// Slot info for each material is always collected. If material import is enabled, asset data is also collected.
// Texture dependencies are also collected if both material and texture import are enabled.
[[nodiscard]]
auto CollectMaterials(aiScene const& scene, bool const import_materials, bool const import_textures,
                      std::vector<SubassetImportSettings>& subasset_settings) -> CollectedMaterials {
  CollectedMaterials ret;
  ret.slots.resize(scene.mNumMaterials);

  if (import_materials) {
    ret.assets.resize(scene.mNumMaterials);
  }

  for (auto i = 0u; i < scene.mNumMaterials; i++) {
    auto const& mtl = *scene.mMaterials[i];

    auto const& mtl_name = mtl.GetName();
    ret.slots[i].name = mtl_name.Empty() ? std::format("Material_{}", i) : std::string{mtl_name.C_Str()};

    if (import_materials) {
      auto const settings_idx = SubassetLayout::GetMaterialSettingsIdx(i);

      if (subasset_settings.size() <= settings_idx) {
        subasset_settings.emplace_back(MaterialImportSettings{}, TextureImportSettings{}, SubassetKind::kMaterial);
      } else if (subasset_settings[settings_idx].kind != SubassetKind::kMaterial) {
        subasset_settings[settings_idx].kind = SubassetKind::kMaterial;
        subasset_settings[settings_idx].material_settings = MaterialImportSettings{};
      }

      // We don't use this yet, but we may want to in the future.
      [[maybe_unused]] auto const& import_settings = subasset_settings[settings_idx].material_settings;

      ret.assets[i] = ExtractMaterial(scene, mtl, import_textures, ret.tex_deps);
    }
  }

  return ret;
}


// Map texture indices must still be raw tex_infos indices, before the fix-up.
// Returns the alpha threshold each texture should preserve coverage for. 0 means no clip material uses it.
[[nodiscard]]
auto ComputeTextureCoverageThresholds(std::span<MaterialAssetData const> const materials,
                                      std::size_t const tex_count) -> std::vector<float> {
  std::vector tex_coverage_thresholds(tex_count, 0.0f);

  for (auto const& mtl : materials) {
    // A factor at or below the threshold clips everything regardless of the texture
    if (mtl.blend_mode != MaterialBlendMode::kAlphaClip || mtl.opacity <= mtl.alpha_threshold) {
      continue;
    }

    // We consider the final opacity mtl opacity * tex opacity, so compute tex threshold here
    auto const tex_threshold = mtl.alpha_threshold / mtl.opacity;

    auto const set_threshold = [&](AssetId const& id) {
      // When materials share a texture, the highest threshold wins: scaling for it errs towards slightly
      // thicker cutouts for the others, rather than letting them thin out faster than without scaling.
      if (auto const idx = id.GetIdxInFile(); idx >= 0) {
        tex_coverage_thresholds[idx] = std::max(tex_coverage_thresholds[idx], tex_threshold);
      }
    };

    set_threshold(mtl.base_color_map);
    set_threshold(mtl.opacity_map);
  }

  return tex_coverage_thresholds;
}


// Converts the raw tex_infos indices of the materials' texture maps into package indices
auto FixUpTextureIds(std::span<MaterialAssetData> const material_data, SubassetLayout const& layout) -> void {
  auto const fix_up_tex_id = [&layout](AssetId& id) {
    if (id.GetIdxInFile() >= 0) {
      id = AssetId{id.GetGuid(), static_cast<int>(layout.GetTextureIdx(id.GetIdxInFile()))};
    }
  };

  for (auto& mtl_data : material_data) {
    fix_up_tex_id(mtl_data.base_color_map);
    fix_up_tex_id(mtl_data.metallic_map);
    fix_up_tex_id(mtl_data.roughness_map);
    fix_up_tex_id(mtl_data.ao_map);
    fix_up_tex_id(mtl_data.normal_map);
    fix_up_tex_id(mtl_data.opacity_map);
    fix_up_tex_id(mtl_data.emissive_map);
  }
}


// Some textures may not be referenced by any material, but we still want to import them.
// Or we could be skipping materials altogether but still want textures.
auto CollectUnreferencedTextures(aiScene const& scene, std::vector<TextureLoadInfo>& tex_infos) -> void {
  for (std::size_t i{0}; i < scene.mNumTextures; ++i) {
    TryEmplaceTexture(tex_infos, *scene.mTextures[i], aiTextureType_UNKNOWN);
  }
}


[[nodiscard]]
auto ImportEmbeddedTextures(std::filesystem::path const& src,
                            std::span<TextureLoadInfo const> const tex_infos,
                            std::span<float const> const tex_coverage_thresholds,
                            SubassetLayout const& layout,
                            std::vector<SubassetImportSettings>& subasset_settings,
                            std::vector<AssetImportResult>& out_results) -> bool {
  for (auto const [idx, tex_info] : tex_infos | std::views::enumerate) {
    auto const subasset_idx{layout.GetTextureSettingsIdx(static_cast<std::size_t>(idx))};

    {
      TextureImportSettings const default_import_settings{
        .type = TextureImportType::kTexture2D,
        .allow_block_compression = true,
        .is_srgb = IsSrgb(tex_info.type),
        .generate_mips = true
      };

      if (subasset_settings.size() <= subasset_idx) {
        subasset_settings.emplace_back(MaterialImportSettings{}, default_import_settings, SubassetKind::kTexture);
      } else if (subasset_settings[subasset_idx].kind != SubassetKind::kTexture) {
        subasset_settings[subasset_idx].kind = SubassetKind::kTexture;
        subasset_settings[subasset_idx].texture_settings = default_import_settings;
      }
    }

    // The texture is compressed
    if (tex_info.tex->mHeight == 0) {
      auto const ext{std::u8string{u8'.'} += reinterpret_cast<char8_t const*>(&tex_info.tex->achFormatHint)};

      TextureImportSource const import_src{
        .file_bytes = std::span{reinterpret_cast<std::byte const*>(tex_info.tex->pcData), tex_info.tex->mWidth},
        .ext = ext
      };

      auto const& saved_settings = subasset_settings[subasset_idx].texture_settings;

      // Resolve Auto into a concrete choice for this import; the saved settings keep Auto
      auto import_settings = saved_settings;

      if (import_settings.alpha_coverage_mode == AlphaCoverageMode::kAuto) {
        auto const derived = std::cmp_less(idx, tex_coverage_thresholds.size()) ? tex_coverage_thresholds[idx] : 0.0f;

        if (derived > 0.0f) {
          import_settings.alpha_coverage_mode = AlphaCoverageMode::kCustom;
          import_settings.alpha_coverage_threshold = derived;
        } else {
          import_settings.alpha_coverage_mode = AlphaCoverageMode::kDisabled;
        }
      }

      auto tex_result{ImportTexture(import_src, import_settings)};

      if (!tex_result) {
        return false;
      }

      auto const tex_name =
        tex_info.tex->mFilename.Empty()
          ? std::string{ToUntypedSv(src.filename().stem().u8string())} += std::format(" Texture {}", idx)
          : std::string{tex_info.tex->mFilename.C_Str()};

      out_results.emplace_back(tex_result->payload_kind, tex_result->runtime_type,
        tex_name, std::move(tex_result->bytes));
    } else {
      // TODO implement this path
      assert("Found uncompressed embedded texture." && false);
      return false;
      //auto& tex_data{
      //  scene_data.textures.emplace_back(tex->mWidth, tex->mHeight,
      //    std::make_unique_for_overwrite<
      //      std::uint8_t[]>(
      //      tex->mWidth * tex->mHeight * 4))
      //};
      //std::memcpy(tex_data.bytes.get(), tex->pcData,
      //  tex->mWidth * tex->mHeight * 4);
    }
  }

  return true;
}


struct NodeProcessingData {
  NotNull<ObserverPtr<aiNode const>> ptr;
  std::optional<std::size_t> parent_idx; // Root node has no parent
};


using NodeListPreorderDfs = std::vector<NodeProcessingData>;


[[nodiscard]]
auto BuildNodeListPreorderDfs(aiScene const& scene) -> NodeListPreorderDfs {
  NodeListPreorderDfs list;

  std::stack<NodeProcessingData> stack;
  stack.emplace(MakeObserver(scene.mRootNode), std::nullopt);

  while (!stack.empty()) {
    auto const node = stack.top();
    stack.pop();

    list.emplace_back(node);

    for (auto const* const child : std::views::reverse(std::span{node.ptr->mChildren, node.ptr->mNumChildren})) {
      // Already placed at least one node in the list, so minus one is safe
      stack.emplace(MakeObserver(child), list.size() - 1);
    }
  }

  return list;
}


// Extracts the vertex attributes, indices and material index of a mesh. Bone weights are left zeroed.
// Returns nullopt if the mesh is missing required vertex attributes.
[[nodiscard]]
auto ExtractAiMesh(aiMesh const& mesh, unsigned idx_in_scene,
                   std::filesystem::path const& src) -> std::optional<MeshProcessingData> {
  // These meshes are always triangle-only, because
  // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines,
  // aiProcess_Triangulate splits up primitives with more than 3 vertices
  // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
  // TODO Implement non-triangle rendering support

  std::string const name{mesh.mName.Empty() ? std::format("Mesh {}", idx_in_scene) : mesh.mName.C_Str()};

  if (!mesh.HasPositions()) {
    OutputDebugStringA(
      std::format("Skipping mesh \"{}\" in file \"{}\" because it is missing vertex positions.\n", name,
        ToUntypedSv(src.u8string())).c_str());
    return std::nullopt;
  }

  if (!mesh.HasNormals()) {
    OutputDebugStringA(
      std::format("Skipping mesh \"{}\" in file \"{}\" because it is missing vertex normals.\n", name,
        ToUntypedSv(src.u8string())).c_str());
    return std::nullopt;
  }

  auto const has_uvs{mesh.HasTextureCoords(0)};

  if (!has_uvs) {
    OutputDebugStringA(
      std::format("Mesh \"{}\" in file \"{}\" is missing texture coordinates. Filling in zeroes.\n", name,
        ToUntypedSv(src.u8string())).c_str());
  }

  auto const has_tangents{mesh.HasTangentsAndBitangents()};

  if (!has_tangents) {
    OutputDebugStringA(
      std::format("Mesh \"{}\" in file \"{}\" is missing tangents. Filling in zeroes.\n", name,
        ToUntypedSv(src.u8string())).c_str());
  }

  MeshProcessingData ret;
  auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{ret};

  vertices.reserve(mesh.mNumVertices);
  normals.reserve(mesh.mNumVertices);
  uvs.reserve(mesh.mNumVertices);
  tangents.reserve(mesh.mNumVertices);
  bone_weights.resize(mesh.mNumVertices);
  bone_indices.resize(mesh.mNumVertices);

  for (unsigned j = 0; j < mesh.mNumVertices; j++) {
    vertices.emplace_back(Convert(mesh.mVertices[j]));
    normals.emplace_back(Normalized(Convert(mesh.mNormals[j])));
    tangents.emplace_back(has_tangents ? Convert(mesh.mTangents[j]) : Vector3{});
    uvs.emplace_back(has_uvs ? Vector2{Convert(mesh.mTextureCoords[0][j])} : Vector2{});
  }

  for (unsigned j = 0; j < mesh.mNumFaces; j++) {
    std::ranges::copy(std::span{mesh.mFaces[j].mIndices, mesh.mFaces[j].mNumIndices},
      std::back_inserter(indices));
  }

  mtlIdx = mesh.mMaterialIndex;

  return ret;
}


// Appends the mesh's bones to bone_proc_info and writes their weights and indices into the mesh data
auto ExtractBoneWeights(aiMesh const& mesh, Matrix4 const& transform_inv,
                        std::vector<BoneProcessingInfo>& bone_proc_info, MeshProcessingData& mesh_data) -> void {
  for (unsigned j{0}; j < mesh.mNumBones; j++) {
    auto const bone{mesh.mBones[j]};

    auto const bone_idx{static_cast<std::uint32_t>(bone_proc_info.size())};
    bone_proc_info.emplace_back(transform_inv * Convert(bone->mOffsetMatrix).Transpose(), bone->mName.C_Str());

    for (unsigned k{0}; k < bone->mNumWeights; k++) {
      auto const& weight{bone->mWeights[k]};

      auto found_free_weight_slot{false};

      for (auto l{0}; l < 4; l++) {
        if (mesh_data.bone_weights[weight.mVertexId][l] == 0.0f) {
          mesh_data.bone_weights[weight.mVertexId][l] = weight.mWeight;
          mesh_data.bone_indices[weight.mVertexId][l] = bone_idx;
          found_free_weight_slot = true;
          break;
        }
      }

      assert(found_free_weight_slot);
    }
  }
}


// Collects every mesh of the scene with its node's absolute transform baked into its vertices
auto CollectBakedMeshes(aiScene const& scene, std::filesystem::path const& src,
                        std::vector<MeshProcessingData>& out_meshes,
                        std::vector<BoneProcessingInfo>& out_bone_proc_info) -> void {
  out_meshes.reserve(scene.mNumMeshes);

  struct NodeAndAccumTrafo {
    Matrix4 absolute_parent_transform;
    aiNode const* node;
  };

  std::queue<NodeAndAccumTrafo> node_transform_queue;
  node_transform_queue.emplace(Matrix4::Identity(), scene.mRootNode);

  while (!node_transform_queue.empty()) {
    auto const& [absolute_parent_transform, node] = node_transform_queue.front();
    auto const abs_transform{Convert(node->mTransformation).Transpose() * absolute_parent_transform};
    auto const abs_transform_inv{abs_transform.Inverse()};
    auto const abs_transform_inv_transp{abs_transform_inv.Transpose()};

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
      auto const mesh_scene_idx = node->mMeshes[i];
      auto const mesh = scene.mMeshes[mesh_scene_idx];

      auto mesh_proc_data = ExtractAiMesh(*mesh, mesh_scene_idx, src);

      if (!mesh_proc_data) {
        continue;
      }

      for (auto j = 0uz; j < mesh_proc_data->positions.size(); ++j) {
        // Position, normal, and tangent lists are guaranteed to be same size

        auto& pos = mesh_proc_data->positions[j];
        pos = Vector3{Vector4{pos, 1} * abs_transform};

        auto& norm = mesh_proc_data->normals[j];
        norm = Vector3{Vector4{norm, 0} * abs_transform_inv_transp};

        auto& tangent = mesh_proc_data->tangents[j];
        tangent = Vector3{Vector4{tangent, 0} * abs_transform_inv_transp};
      }

      ExtractBoneWeights(*mesh, abs_transform_inv, out_bone_proc_info, *mesh_proc_data);
      out_meshes.emplace_back(std::move(*mesh_proc_data));
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      node_transform_queue.emplace(abs_transform, node->mChildren[i]);
    }

    node_transform_queue.pop();
  }
}


// Collects every mesh of the scene without any modification
[[nodiscard]]
auto CollectRawMeshes(aiScene const& scene,
                      std::filesystem::path const& src) -> std::vector<std::optional<MeshProcessingData>> {
  std::vector<std::optional<MeshProcessingData>> meshes(scene.mNumMeshes);

  for (auto i = 0u; i < scene.mNumMeshes; ++i) {
    meshes[i] = ExtractAiMesh(*scene.mMeshes[i], i, src);
  }

  return meshes;
}


using MeshIndex = std::size_t;
using NodeIndex = std::size_t;
using NodesByMesh = std::vector<std::vector<NodeIndex>>;

// SplitMix64 finalizer: a bijective 64-bit mixer with strong avalanche.
constexpr auto Mix64(std::uint64_t x) noexcept -> std::uint64_t {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}


struct NodeListHash {
  auto operator()(std::span<NodeIndex const> const nodes) const noexcept -> std::size_t {
    auto h = Mix64(nodes.size());

    for (NodeIndex const n : nodes) {
      h = Mix64(h ^ n);
    }

    return h;
  }
};


struct NodeListEq {
  auto operator()(std::span<NodeIndex const> const lhs, std::span<NodeIndex const> const rhs) const noexcept -> bool {
    return std::ranges::equal(lhs, rhs);
  }
};


[[nodiscard]]
auto CollectReferringNodesPerMesh(std::span<NodeProcessingData const> const nodes,
                                  std::size_t const mesh_count) -> NodesByMesh {
  NodesByMesh nodes_by_mesh(mesh_count);

  for (auto const [node_idx, node] : std::views::enumerate(nodes)) {
    for (auto j = 0u; j < node.ptr->mNumMeshes; ++j) {
      // Guard against the case where a node lists the same mesh multiple times
      if (auto& this_mesh_nodes = nodes_by_mesh[node.ptr->mMeshes[j]];
        this_mesh_nodes.empty() || std::cmp_not_equal(this_mesh_nodes.back(), node_idx)) {
        this_mesh_nodes.emplace_back(node_idx);
      }
    }
  }

  return nodes_by_mesh;
}


using MeshGroupIndex = std::size_t;
using MeshesByGroup = std::vector<std::vector<MeshIndex>>;


[[nodiscard]]
auto CollectMeshesByGroup(NodesByMesh const& nodes_by_mesh,
                          std::span<std::optional<MeshProcessingData> const> const meshes) -> MeshesByGroup {
  std::unordered_map<std::span<NodeIndex const>, MeshGroupIndex, NodeListHash, NodeListEq> groups_by_node_list;
  MeshesByGroup meshes_by_group;

  for (auto const [mesh_idx, node_list] : std::views::enumerate(nodes_by_mesh)) {
    // Meshes that failed to convert are just simply not added to any group
    // Saves us some extra bookkeeping later
    if (!meshes[mesh_idx]) {
      continue;
    }

    auto const [it, inserted] = groups_by_node_list.try_emplace(node_list, meshes_by_group.size());

    // If inserted, the group / node list pattern is first visited, so create a new group for it
    if (inserted) {
      meshes_by_group.emplace_back();
    }

    meshes_by_group[it->second].emplace_back(mesh_idx);
  }

  return meshes_by_group;
}


using GroupsByNode = std::vector<std::vector<MeshGroupIndex>>;


[[nodiscard]]
auto CollectGroupsByNode(MeshesByGroup const& meshes_by_group, NodesByMesh const& nodes_by_mesh,
                         std::size_t const node_count) -> GroupsByNode {
  GroupsByNode groups_by_node(node_count);

  for (auto const [group_idx, mesh_indices] : std::views::enumerate(meshes_by_group)) {
    // For each node, we generate the list of groups that the node references.
    // All meshes in the same group have the same node pattern so it doesnt matter which one we use, front is good.
    for (auto const node_idx : nodes_by_mesh[mesh_indices.front()]) {
      groups_by_node[node_idx].emplace_back(group_idx);
    }
  }

  return groups_by_node;
}


using SceneMaterialIndex = std::size_t;
using MaterialSlotsByMesh = std::vector<std::vector<SceneMaterialIndex>>;


// Fill mesh material slots and remap submesh material slot indices to point into the mesh-local slots.
// A table that describes the remap is returned.
[[nodiscard]]
auto FixUpMaterialIds(std::span<MeshData> const imported_meshes,
                      std::span<MaterialSlotInfo const> const mtl_slots) -> MaterialSlotsByMesh {
  MaterialSlotsByMesh mtl_slots_by_mesh(imported_meshes.size());

  for (auto [mesh_idx, mesh] : std::views::enumerate(imported_meshes)) {
    auto& slots = mtl_slots_by_mesh[mesh_idx];

    for (auto const& submesh : mesh.submeshes) {
      slots.emplace_back(submesh.material_idx);
    }

    std::ranges::sort(slots);
    auto const [first, last] = std::ranges::unique(slots);
    slots.erase(first, last);

    mesh.material_slots.reserve(slots.size());

    for (auto const slot_idx : slots) {
      mesh.material_slots.emplace_back(mtl_slots[slot_idx].name);
    }

    for (auto& submesh : mesh.submeshes) {
      auto const it = std::ranges::lower_bound(slots, submesh.material_idx);
      auto const remapped_idx = std::ranges::distance(slots.begin(), it);
      submesh.material_idx = static_cast<std::uint32_t>(remapped_idx);
    }
  }

  return mtl_slots_by_mesh;
}


// Builds the skeleton from the bones' nodes and all of their ancestors, then creates the final bone data.
// Returns the skeleton node index of each skeleton node name.
[[nodiscard]]
auto BuildSkeleton(aiNode const& root, std::span<BoneProcessingInfo const> const bone_proc_info,
                   std::vector<SkeletonNode>& out_skeleton,
                   std::vector<Bone>& out_bones) -> std::unordered_map<std::string, std::uint32_t> {
  // Convert node hierarchy

  auto root_node{Convert(&root, nullptr)};

  // Mark nodes that are part of the skeleton (but not necessarily bones!)

  std::ranges::for_each(bone_proc_info, [&root_node](BoneProcessingInfo const& bone_info) {
    // We look for the node corresponding to the bone and mark it along with all of its ancestors
    auto node{FindInHierarchyByName(*root_node, bone_info.node_name)};
    assert(node);

    while (node) {
      node->visited = true;
      node = node->parent;
    }
  });

  // Flatten visited hierarchy to a BFS list

  // Indices are needed for node animations. It is guaranteed that animated nodes have unique names.
  std::unordered_map<std::string, std::uint32_t> skeleton_node_name_to_idx;

  struct NodeAndParentIdx {
    Node* node;
    std::optional<std::uint32_t> parent_idx;
  };

  std::queue<NodeAndParentIdx> node_queue;

  if (!root_node->visited) {
    root_node.reset();
  } else {
    node_queue.emplace(root_node.get(), std::nullopt);
  }

  while (!node_queue.empty()) {
    auto const& [node, parent_idx]{node_queue.front()};
    auto const this_node_idx{static_cast<std::uint32_t>(out_skeleton.size())};
    out_skeleton.emplace_back(node->name, node->transform, parent_idx);
    skeleton_node_name_to_idx.try_emplace(node->name, this_node_idx);

    for (auto const& child : node->children) {
      if (child->visited) {
        node_queue.emplace(child.get(), this_node_idx);
      }
    }

    node_queue.pop();
  }

  // Create final bone data

  out_bones.reserve(bone_proc_info.size());
  std::ranges::transform(bone_proc_info, std::back_inserter(out_bones),
    [&skeleton_node_name_to_idx](BoneProcessingInfo const& bone_info) {
      return Bone{
        .offset_mtx = bone_info.offset_matrix, .skeleton_node_idx = skeleton_node_name_to_idx.at(bone_info.node_name)
      };
    });

  return skeleton_node_name_to_idx;
}


// Fuses meshes that share materials
[[nodiscard]]
auto FuseByMaterial(std::span<MeshProcessingData const> const meshes) -> std::vector<MeshProcessingData> {
  std::map<std::uint32_t, std::vector<MeshProcessingData const*>> meshes_per_mtl_idx;

  for (auto const& mesh : meshes) {
    meshes_per_mtl_idx[mesh.mtl_idx].push_back(&mesh);
  }

  std::vector<MeshProcessingData> fused_meshes;
  fused_meshes.reserve(meshes_per_mtl_idx.size());

  for (auto const& [mtl_idx, mtl_meshes] : meshes_per_mtl_idx) {
    auto& fused_mesh{fused_meshes.emplace_back()};

    fused_mesh.mtl_idx = mtl_idx;

    for (auto const& mesh : mtl_meshes) {
      // Bake 'baseVertex' into indices
      fused_mesh.indices.reserve(fused_mesh.indices.size() + mesh->indices.size());
      std::ranges::transform(mesh->indices, std::back_inserter(fused_mesh.indices),
        [&fused_mesh](unsigned const idx) {
          return static_cast<unsigned>(idx + fused_mesh.positions.size());
        });

      fused_mesh.positions.reserve(fused_mesh.positions.size() + mesh->positions.size());
      std::ranges::copy(mesh->positions, std::back_inserter(fused_mesh.positions));

      fused_mesh.normals.reserve(fused_mesh.normals.size() + mesh->normals.size());
      std::ranges::copy(mesh->normals, std::back_inserter(fused_mesh.normals));

      fused_mesh.tangents.reserve(fused_mesh.tangents.size() + mesh->tangents.size());
      std::ranges::copy(mesh->tangents, std::back_inserter(fused_mesh.tangents));

      fused_mesh.uvs.reserve(fused_mesh.uvs.size() + mesh->uvs.size());
      std::ranges::copy(mesh->uvs, std::back_inserter(fused_mesh.uvs));

      fused_mesh.bone_weights.reserve(fused_mesh.bone_weights.size() + mesh->bone_weights.size());
      std::ranges::copy(mesh->bone_weights, std::back_inserter(fused_mesh.bone_weights));

      fused_mesh.bone_indices.reserve(fused_mesh.bone_indices.size() + mesh->bone_indices.size());
      std::ranges::copy(mesh->bone_indices, std::back_inserter(fused_mesh.bone_indices));
    }
  }

  return fused_meshes;
}


template<class R, class T>
concept SizedRangeOf =
  std::ranges::forward_range<R> &&
  std::ranges::sized_range<R> &&
  std::same_as<std::ranges::range_const_reference_t<R>, T const&>;


// Meshletizes the meshes and appends them to the mesh data as submeshes, one per mesh.
// Fills in the geometry, the submeshes, the bounds and the index format.
[[nodiscard]]
auto BuildMeshGeometry(SizedRangeOf<MeshProcessingData const> auto&& meshes, bool const force_idx32,
                       JobSystem& job_system, MeshData& mesh_data) -> bool {
  // Determine index format

  mesh_data.idx32 = force_idx32 || std::ranges::any_of(meshes,
                      [](MeshProcessingData const& mesh) {
                        return std::ranges::any_of(mesh.indices, [](unsigned const idx) {
                          return idx > std::numeric_limits<std::uint16_t>::max();
                        });
                      });

  // Meshletize submeshes

  struct MeshletizedMesh {
    std::vector<MeshletData> meshlets;
    std::vector<std::uint8_t> unique_vertex_indices;
    std::vector<MeshletTriangleData> primitive_indices;
    std::vector<MeshletCullData> cull_data;
  };

  std::vector<std::optional<MeshletizedMesh>> meshletized_meshes(std::ranges::size(meshes));
  std::vector<ObserverPtr<Job>> meshletize_jobs;
  meshletize_jobs.reserve(std::ranges::size(meshes));

  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    meshletize_jobs.emplace_back(job_system.CreateJob(
      [i, &mesh, &meshletized_meshes, idx32 = mesh_data.idx32] {
        auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtl_idx]{mesh};
        auto [meshlets, unique_vertex_indices, primitive_indices, cull_data]{MeshletizedMesh{}};

        bool success;

        if (idx32) {
          success = ComputeMeshlets<std::uint32_t, Vector3>(indices, positions, meshlets, unique_vertex_indices,
            primitive_indices, cull_data);
        } else {
          std::vector<std::uint16_t> indices16(indices.size());
          std::ranges::transform(indices, indices16.begin(), [](unsigned const idx) {
            return static_cast<std::uint16_t>(idx);
          });

          success = ComputeMeshlets<std::uint16_t, Vector3>(indices16, positions, meshlets, unique_vertex_indices,
            primitive_indices, cull_data);
        }

        if (success) {
          meshletized_meshes[i] = MeshletizedMesh{
            std::move(meshlets), std::move(unique_vertex_indices),
            std::move(primitive_indices), std::move(cull_data)
          };
        }
      }));

    job_system.Submit(meshletize_jobs.back());
  }

  job_system.WaitAll(meshletize_jobs);

  // Combine geometry

  std::vector<SubmeshMeshletRange> submesh_meshlet_ranges;
  submesh_meshlet_ranges.reserve(std::ranges::size(meshes));

  std::vector<AABB> submesh_bounds;
  submesh_bounds.reserve(std::ranges::size(meshes));

  std::vector<std::uint32_t> submesh_base_vertices;
  submesh_base_vertices.reserve(std::ranges::size(meshes));

  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    if (!meshletized_meshes[i]) {
      DisplayError(std::format("Failed to compute meshlets for submesh {}.", i));
      return false;
    }

    auto& [meshlets, unique_vertex_indices, primitive_indices, cull_data]{*meshletized_meshes[i]};
    auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{mesh};

    submesh_base_vertices.emplace_back(static_cast<std::uint32_t>(mesh_data.positions.size()));

    submesh_meshlet_ranges.emplace_back(mesh_data.meshlets.size(), meshlets.size());

    mesh_data.meshlets.reserve(mesh_data.meshlets.size() + meshlets.size());
    std::ranges::transform(meshlets, std::back_inserter(mesh_data.meshlets),
      [&mesh_data](MeshletData const& meshlet) {
        // Offset index ranges by existing index counts
        return MeshletData{
          .vert_count = meshlet.vert_count,
          .vert_offset = meshlet.vert_offset + static_cast<std::uint32_t>(
                           mesh_data.vertex_indices.size() / (mesh_data.idx32 ? 4 : 2)),
          .prim_count = meshlet.prim_count,
          .prim_offset = meshlet.prim_offset + static_cast<std::uint32_t>(mesh_data.triangle_indices.size()),
        };
      });

    submesh_bounds.emplace_back(AABB::FromVertices(positions));

    mesh_data.positions.reserve(mesh_data.positions.size() + positions.size());
    std::ranges::copy(positions, std::back_inserter(mesh_data.positions));

    mesh_data.normals.reserve(mesh_data.normals.size() + normals.size());
    std::ranges::copy(normals, std::back_inserter(mesh_data.normals));

    mesh_data.tangents.reserve(mesh_data.tangents.size() + tangents.size());
    std::ranges::copy(tangents, std::back_inserter(mesh_data.tangents));

    mesh_data.uvs.reserve(mesh_data.uvs.size() + uvs.size());
    std::ranges::copy(uvs, std::back_inserter(mesh_data.uvs));

    mesh_data.bone_weights.reserve(mesh_data.bone_weights.size() + bone_weights.size());
    std::ranges::copy(bone_weights, std::back_inserter(mesh_data.bone_weights));

    mesh_data.bone_indices.reserve(mesh_data.bone_indices.size() + bone_indices.size());
    std::ranges::copy(bone_indices, std::back_inserter(mesh_data.bone_indices));

    mesh_data.vertex_indices.reserve(mesh_data.vertex_indices.size() + unique_vertex_indices.size());
    std::ranges::copy(unique_vertex_indices, std::back_inserter(mesh_data.vertex_indices));

    mesh_data.triangle_indices.reserve(mesh_data.triangle_indices.size() + primitive_indices.size());
    std::ranges::copy(primitive_indices, std::back_inserter(mesh_data.triangle_indices));

    mesh_data.cull_data.reserve(mesh_data.cull_data.size() + cull_data.size());
    std::ranges::copy(cull_data, std::back_inserter(mesh_data.cull_data));
  }

  // Create and store submeshes

  mesh_data.submeshes.reserve(std::ranges::size(meshes));
  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    mesh_data.submeshes.emplace_back(SubmeshData{
      .first_meshlet = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].first_meshlet),
      .meshlet_count = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].meshlet_count),
      .base_vertex = submesh_base_vertices[i],
      .material_idx = mesh.mtl_idx,
      .bounds = submesh_bounds[i],
    });
  }

  // Bounds

  mesh_data.bounds = AABB::FromVertices(mesh_data.positions);

  return true;
}


// Collects the animations of skeleton nodes. Channels animating other nodes are skipped.
[[nodiscard]]
auto CollectAnimations(aiScene const& scene,
                       std::unordered_map<std::string, std::uint32_t> const& skeleton_node_name_to_idx) -> std::vector<
  Animation> {
  std::vector<Animation> animations;
  animations.reserve(scene.mNumAnimations);

  for (unsigned i{0}; i < scene.mNumAnimations; i++) {
    auto const anim{scene.mAnimations[i]};

    std::vector<NodeAnimation> node_anims;

    for (unsigned j{0}; j < anim->mNumChannels; j++) {
      auto const channel{anim->mChannels[j]};

      // Check if the animated node is an ancestor of a mesh's node

      std::queue<aiNode const*> node_search_queue;
      node_search_queue.emplace(scene.mRootNode);

      std::queue<aiNode const*> mesh_search_queue;

      // Find the node that the channel is animating
      while (!node_search_queue.empty()) {
        auto const node{node_search_queue.front()};

        if (node->mName == channel->mNodeName) {
          mesh_search_queue.emplace(node);
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          node_search_queue.emplace(node->mChildren[k]);
        }

        node_search_queue.pop();
      }

      // Check if any of the node's descendants are mesh nodes
      while (!mesh_search_queue.empty()) {
        auto const node{mesh_search_queue.front()};

        if (node->mNumMeshes > 0) {
          // TODO log this properly
          OutputDebugStringA(
            "Animated node is an ancestor of a submesh node. Animations on submesh nodes are currently not supported.\n");
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          mesh_search_queue.emplace(node->mChildren[k]);
        }

        mesh_search_queue.pop();
      }

      // Check if the animated node is part of the skeleton

      if (!skeleton_node_name_to_idx.contains(channel->mNodeName.C_Str())) {
        // TODO log this properly
        OutputDebugStringA("Animated node is not part of the skeleton. Such animations are currently not supported.\n");
        continue;
      }

      std::vector<AnimPositionKey> position_keys;
      position_keys.reserve(channel->mNumPositionKeys);
      std::ranges::transform(channel->mPositionKeys, channel->mPositionKeys + channel->mNumPositionKeys,
        std::back_inserter(position_keys), [](aiVectorKey const& pos_key) {
          return AnimPositionKey{
            static_cast<float>(pos_key.mTime), Convert(pos_key.mValue)
          };
        });

      std::vector<AnimRotationKey> rotation_keys;
      rotation_keys.reserve(channel->mNumRotationKeys);
      std::ranges::transform(channel->mRotationKeys, channel->mRotationKeys + channel->mNumRotationKeys,
        std::back_inserter(rotation_keys), [](aiQuatKey const& rot_key) {
          return AnimRotationKey{
            static_cast<float>(rot_key.mTime), Convert(rot_key.mValue)
          };
        });

      std::vector<AnimScalingKey> scaling_keys;
      scaling_keys.reserve(channel->mNumScalingKeys);
      std::ranges::transform(channel->mScalingKeys, channel->mScalingKeys + channel->mNumScalingKeys,
        std::back_inserter(scaling_keys), [](aiVectorKey const& scale_key) {
          return AnimScalingKey{
            static_cast<float>(scale_key.mTime), Convert(scale_key.mValue)
          };
        });

      node_anims.emplace_back(std::move(position_keys), std::move(rotation_keys), std::move(scaling_keys),
        skeleton_node_name_to_idx.at(channel->mNodeName.C_Str()));
    }

    animations.emplace_back(anim->mName.C_Str(), static_cast<float>(anim->mDuration),
      static_cast<float>(anim->mTicksPerSecond), std::move(node_anims));
  }

  return animations;
}


[[nodiscard]]
auto SerializeMesh(MeshData const& mesh_data) -> std::vector<std::byte> {
  std::vector<std::byte> bytes;

  // Element counts

  SerializeToBinary(mesh_data.positions.size(), bytes);
  SerializeToBinary(mesh_data.meshlets.size(), bytes);
  SerializeToBinary(mesh_data.vertex_indices.size(), bytes);
  SerializeToBinary(mesh_data.triangle_indices.size(), bytes);
  SerializeToBinary(mesh_data.material_slots.size(), bytes);
  SerializeToBinary(mesh_data.submeshes.size(), bytes);
  SerializeToBinary(mesh_data.animations.size(), bytes);
  SerializeToBinary(mesh_data.skeleton.size(), bytes);
  SerializeToBinary(mesh_data.bones.size(), bytes);

  // Geometry data

  auto const pos_bytes{as_bytes(std::span{mesh_data.positions})};
  auto const norm_bytes{as_bytes(std::span{mesh_data.normals})};
  auto const tan_bytes{as_bytes(std::span{mesh_data.tangents})};
  auto const uv_bytes{as_bytes(std::span{mesh_data.uvs})};
  auto const bone_weight_bytes{as_bytes(std::span{mesh_data.bone_weights})};
  auto const bone_idx_bytes{as_bytes(std::span{mesh_data.bone_indices})};
  auto const meshlet_bytes{as_bytes(std::span{mesh_data.meshlets})};
  auto const vtx_idx_bytes{as_bytes(std::span{mesh_data.vertex_indices})};
  auto const prim_idx_bytes{as_bytes(std::span{mesh_data.triangle_indices})};
  auto const cull_data_bytes{as_bytes(std::span{mesh_data.cull_data})};

  bytes.reserve(
    std::size(bytes) + std::size(pos_bytes) + std::size(norm_bytes) + std::size(tan_bytes) + std::size(uv_bytes) +
    std::size(bone_weight_bytes) + std::size(bone_idx_bytes) + std::size(meshlet_bytes) + std::size(vtx_idx_bytes) +
    std::size(prim_idx_bytes) + std::size(cull_data_bytes));

  std::ranges::copy(pos_bytes, std::back_inserter(bytes));
  std::ranges::copy(norm_bytes, std::back_inserter(bytes));
  std::ranges::copy(tan_bytes, std::back_inserter(bytes));
  std::ranges::copy(uv_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_weight_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(meshlet_bytes, std::back_inserter(bytes));
  std::ranges::copy(vtx_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(prim_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(cull_data_bytes, std::back_inserter(bytes));

  // Material slots

  for (auto const& mtl_slot : mesh_data.material_slots) {
    SerializeToBinary(mtl_slot.name, bytes);
  }

  // Submeshes

  for (auto const& submesh : mesh_data.submeshes) {
    SerializeToBinary(submesh.first_meshlet, bytes);
    SerializeToBinary(submesh.meshlet_count, bytes);
    SerializeToBinary(submesh.base_vertex, bytes);
    SerializeToBinary(submesh.material_idx, bytes);

    for (auto i{0}; i < 3; i++) {
      SerializeToBinary(submesh.bounds.min[i], bytes);
    }

    for (auto i{0}; i < 3; i++) {
      SerializeToBinary(submesh.bounds.max[i], bytes);
    }
  }

  // Animations

  for (auto const& [name, duration, ticks_per_second, node_anims] : mesh_data.animations) {
    SerializeToBinary(name, bytes);
    SerializeToBinary(duration, bytes);
    SerializeToBinary(ticks_per_second, bytes);
    SerializeToBinary(node_anims.size(), bytes);

    for (auto const& [position_keys, rotation_keys, scaling_keys, node_idx] : node_anims) {
      SerializeToBinary(node_idx, bytes);

      SerializeToBinary(position_keys.size(), bytes);
      SerializeToBinary(rotation_keys.size(), bytes);
      SerializeToBinary(scaling_keys.size(), bytes);

      auto const pos_key_bytes{as_bytes(std::span{position_keys})};
      auto const rot_key_bytes{as_bytes(std::span{rotation_keys})};
      auto const scale_key_bytes{as_bytes(std::span{scaling_keys})};

      bytes.reserve(pos_key_bytes.size() + rot_key_bytes.size() + scale_key_bytes.size());

      std::ranges::copy(pos_key_bytes, std::back_inserter(bytes));
      std::ranges::copy(rot_key_bytes, std::back_inserter(bytes));
      std::ranges::copy(scale_key_bytes, std::back_inserter(bytes));
    }
  }

  // Skeleton nodes

  for (auto const& [name, transform, parent_idx] : mesh_data.skeleton) {
    SerializeToBinary(name, bytes);
    SerializeToBinary(parent_idx.has_value(), bytes);

    if (parent_idx) {
      SerializeToBinary(*parent_idx, bytes);
    }

    std::ranges::copy(as_bytes(std::span{transform.GetData(), 16}), std::back_inserter(bytes));
  }

  // Bones

  for (auto const& [offset_matrix, node_idx] : mesh_data.bones) {
    std::ranges::copy(as_bytes(std::span{offset_matrix.GetData(), 16}), std::back_inserter(bytes));
    SerializeToBinary(node_idx, bytes);
  }

  // Bounds

  for (auto i{0}; i < 3; i++) {
    SerializeToBinary(mesh_data.bounds.min[i], bytes);
  }

  for (auto i{0}; i < 3; i++) {
    SerializeToBinary(mesh_data.bounds.max[i], bytes);
  }

  // Index format

  SerializeToBinary(mesh_data.idx32, bytes);

  return bytes;
}


[[nodiscard]]
auto SerializePrefab(std::span<std::unique_ptr<Entity> const> const entities) -> std::vector<std::byte> {
  auto entity_ptrs = entities |
                     std::views::transform([](auto const& ptr) { return ptr.get(); }) |
                     std::ranges::to<std::vector<Entity const*>>();

  auto const prefab_node{
    SerializeEntitySet(entity_ptrs, EntitySerializationContext{
      .asset_ref_serialization = AssetRefSerialization::kLocal
    })
  };

  auto const yaml_str{YAML::Dump(prefab_node)};

  std::vector<std::byte> prefab_bytes;
  prefab_bytes.resize(yaml_str.size());
  std::ranges::copy(yaml_str | std::views::transform([](char c) { return static_cast<std::byte>(c); }),
    prefab_bytes.begin());

  return prefab_bytes;
}


// Creates a prefab of a single entity that renders the mesh with the materials.
// Placeholder mesh and material objects carry the package indices the prefab's asset references serialize to.
[[nodiscard]]
auto CreateSingleEntityPrefab(std::string_view const name, MeshData const& mesh,
                              SubassetLayout const& layout,
                              std::span<SceneMaterialIndex const> const mesh_mtl_slot_remap) -> AssetImportResult {
  auto const entity{std::make_unique<Entity>()};
  entity->SetName(name);

  auto mesh_component{std::make_unique<StaticMeshComponent>()};

  auto const dummy_mesh = std::make_unique<Mesh>(mesh, CpuResidencyPolicy::kKeepResident);
  dummy_mesh->SetAssetId(AssetId{Guid::Invalid(), static_cast<int>(layout.GetMeshIdx(0))});

  mesh_component->SetMesh(MakeAssetRef(MakeObserver(dummy_mesh.get())));

  std::vector<std::unique_ptr<Material>> dummy_materials;
  dummy_materials.reserve(layout.GetMaterialCount());

  for (auto i = 0uz; i < layout.GetMaterialCount(); i++) {
    auto dummy_mtl{std::make_unique<Material>()};
    dummy_mtl->SetAssetId(AssetId{Guid::Invalid(), static_cast<int>(layout.GetMaterialIdx(i))});
    dummy_materials.emplace_back(std::move(dummy_mtl));
  }

  if (!dummy_materials.empty()) {
    for (auto i = 0uz; i < mesh.material_slots.size(); ++i) {
      mesh_component->SetMaterial(static_cast<int>(i),
        MakeAssetRef(MakeObserver(dummy_materials.at(mesh_mtl_slot_remap[i]).get())));
    }
  }

  entity->AddComponent(std::move(mesh_component));

  return AssetImportResult{
    AssetPackagePayloadKind::kPrefab, rttr::type::get<Prefab>(), std::string{entity->GetName()},
    SerializePrefab(std::span{&entity, 1})
  };
}


[[nodiscard]]
auto CreatePrefab(
  std::string_view const name,
  std::span<MeshData const> const mesh_groups,
  GroupsByNode const& groups_by_node,
  MaterialSlotsByMesh const& mtl_slots_by_mesh,
  NodeListPreorderDfs const& node_list,
  SubassetLayout const& layout
) -> AssetImportResult {
  // Create dummies for all mesh groups

  std::vector<std::unique_ptr<Mesh>> dummy_meshes(mesh_groups.size());
  for (auto const [group_idx, group] : std::views::enumerate(mesh_groups)) {
    // The only thing we care about in a dummy mesh is its material slot count
    MeshData const mesh_data{
      .positions = {},
      .normals = {},
      .tangents = {},
      .uvs = {},
      .bone_weights = {},
      .bone_indices = {},
      .meshlets = {},
      .vertex_indices = {},
      .triangle_indices = {},
      .cull_data = {},
      .material_slots = mesh_groups[group_idx].material_slots,
      .submeshes = {},
      .animations = {},
      .skeleton = {},
      .bones = {},
      .bounds = {},
      .idx32 = {}
    };

    auto dummy_mesh = std::make_unique<Mesh>(mesh_data, CpuResidencyPolicy::kKeepResident);
    dummy_mesh->SetAssetId(AssetId{Guid::Invalid(), static_cast<int>(layout.GetMeshIdx(group_idx))});
    dummy_meshes[group_idx] = std::move(dummy_mesh);
  }

  // Create dummies for all materials

  std::vector<std::unique_ptr<Material>> dummy_materials(layout.GetMaterialCount());
  for (auto i = 0uz; i < layout.GetMaterialCount(); ++i) {
    auto dummy_mtl = std::make_unique<Material>();
    dummy_mtl->SetAssetId(AssetId{Guid::Invalid(), static_cast<int>(layout.GetMaterialIdx(i))});
    dummy_materials[i] = std::move(dummy_mtl);
  }

  // Create the entity hierarchy

  std::vector<std::unique_ptr<Entity>> entities(node_list.size());

  for (auto const [node_idx, node] : std::views::enumerate(node_list)) {
    auto entity = std::make_unique<Entity>();

    auto const entity_name = node.ptr->mName.Empty()
                               ? std::format("Node {}", node_idx)
                               : std::string{node.ptr->mName.C_Str()};
    entity->SetName(entity_name);

    aiVector3f position;
    aiQuaternion rotation;
    aiVector3f scaling;
    node.ptr->mTransformation.Decompose(scaling, rotation, position);

    auto& transform = entity->GetTransform();
    transform.SetLocalPosition(Convert(position));
    transform.SetLocalRotation(Convert(rotation));
    transform.SetLocalScale(Convert(scaling));

    if (node.parent_idx) {
      transform.SetParent(&entities[*node.parent_idx]->GetTransform());
    }

    for (auto const& group_idx : groups_by_node[node_idx]) {
      auto mesh_component{std::make_unique<StaticMeshComponent>()};

      mesh_component->SetMesh(MakeAssetRef(MakeObserver(dummy_meshes[group_idx].get())));

      if (!dummy_materials.empty()) {
        for (auto i = 0uz; i < mesh_groups[group_idx].material_slots.size(); ++i) {
          mesh_component->SetMaterial(static_cast<int>(i),
            MakeAssetRef(MakeObserver(dummy_materials.at(mtl_slots_by_mesh.at(group_idx).at(i)).get())));
        }
      }

      entity->AddComponent(std::move(mesh_component));
    }

    entities[node_idx] = std::move(entity);
  }

  if (!entities.empty()) {
    // Rename the root of the prefab to the prefab asset name
    entities[0]->SetName(name);
  }

  return AssetImportResult{
    AssetPackagePayloadKind::kPrefab, rttr::type::get<Prefab>(), std::string{name},
    SerializePrefab(entities)
  };
}


auto ImportMaterials(std::span<MaterialAssetData const> const material_data,
                     std::span<MaterialSlotInfo const> const material_slots,
                     std::vector<AssetImportResult>& out_results) -> void {
  for (std::size_t i{0}; i < material_data.size(); i++) {
    auto mtl_result{ImportMaterial(material_data[i], {})};
    out_results.emplace_back(mtl_result.payload_kind, mtl_result.runtime_type, material_slots[i].name,
      std::move(mtl_result.bytes));
  }
}
}


auto ModelImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  std::string extensions;
  Assimp::Importer const importer;
  importer.GetExtensionList(extensions);

  // Assimp extension list format is "*.3ds;*.obj;*.dae"
  for (auto const ext : std::views::split(extensions, std::string_view{";"})) {
    out.emplace_back(std::string_view{std::begin(ext), std::end(ext)}.substr(1));
  }
}


auto ModelImporter::Import(std::filesystem::path const& src, std::vector<AssetImportResult>& results) -> bool {
  auto const importer{LoadScene(src)};

  if (!importer) {
    return false;
  }

  auto const& scene{*importer->GetScene()};

  // Import meshes

  auto const import_as_skinned_mesh = std::ranges::any_of(std::span{scene.mMeshes, scene.mNumMeshes},
    [](auto const* const mesh) { return mesh->HasBones(); });

  std::vector<MeshData> imported_meshes;
  std::vector<NodeProcessingData> nodes;
  NodesByMesh nodes_by_mesh;
  MeshesByGroup meshes_by_group;
  GroupsByNode groups_by_node;

  if (import_as_skinned_mesh) {
    auto& mesh_data = imported_meshes.emplace_back();

    std::vector<MeshProcessingData> meshes;
    std::vector<BoneProcessingInfo> bone_proc_info;
    CollectBakedMeshes(scene, src, meshes, bone_proc_info);

    auto const skeleton_node_name_to_idx = BuildSkeleton(*scene.mRootNode, bone_proc_info, mesh_data.skeleton,
      mesh_data.bones);

    if (mesh_import_settings_.fuse_submeshes) {
      meshes = FuseByMaterial(meshes);
    }

    if (!BuildMeshGeometry(meshes, mesh_import_settings_.force_idx32, App::Instance().GetJobSystem(), mesh_data)) {
      return false;
    }

    mesh_data.animations = CollectAnimations(scene, skeleton_node_name_to_idx);
  } else {
    auto const meshes = CollectRawMeshes(scene, src);
    nodes = BuildNodeListPreorderDfs(scene);
    nodes_by_mesh = CollectReferringNodesPerMesh(nodes, meshes.size());
    meshes_by_group = CollectMeshesByGroup(nodes_by_mesh, meshes);
    groups_by_node = CollectGroupsByNode(meshes_by_group, nodes_by_mesh, nodes.size());

    for (auto const [group_idx, mesh_indices] : std::views::enumerate(meshes_by_group)) {
      auto const group_meshes = mesh_indices | std::views::transform([&meshes](auto const mesh_idx) -> auto const& {
        return meshes[mesh_idx].value();
      });

      MeshData mesh_data;

      if (!BuildMeshGeometry(group_meshes, mesh_import_settings_.force_idx32, App::Instance().GetJobSystem(),
        mesh_data)) {
        return false;
      }

      imported_meshes.emplace_back(std::move(mesh_data));
    }
  }

  // Import materials and gather info to load textures

  auto [mtl_slots, mtl_assets, texes_to_load] = CollectMaterials(scene, import_materials_, import_textures_,
    subasset_import_settings_);
  auto const tex_coverage_thresholds = ComputeTextureCoverageThresholds(mtl_assets, texes_to_load.size());

  //  At this point, the submeshes inside each imported mesh have absolute material indices, not local to their parent mesh.
  // We can fix those up now and generate a table that describes the remap. Useful later.
  auto const mtl_slots_by_mesh = FixUpMaterialIds(imported_meshes, mtl_slots);

  // Now we know how many meshes and materials we have so patch material->texture references
  SubassetLayout const layout{imported_meshes.size(), mtl_assets.size()};
  FixUpTextureIds(mtl_assets, layout);

  // Nothing depends on the actual texture assets so we can load them last

  std::vector<AssetImportResult> texture_import_results;

  if (import_textures_) {
    CollectUnreferencedTextures(scene, texes_to_load);

    if (!ImportEmbeddedTextures(src, texes_to_load, tex_coverage_thresholds, layout, subasset_import_settings_,
      texture_import_results)) {
      return false;
    }
  }

  // Write subassets to output in package order

  if (import_as_skinned_mesh) {
    results.emplace_back(CreateSingleEntityPrefab(ToUntypedSv(src.filename().stem().u8string()), imported_meshes.at(0),
      layout, mtl_slots_by_mesh.at(0)));
  } else {
    results.emplace_back(CreatePrefab(ToUntypedSv(src.filename().stem().u8string()), imported_meshes, groups_by_node,
      mtl_slots_by_mesh, nodes, layout));
  }

  for (auto const& mesh_data : imported_meshes) {
    auto mesh_bytes = SerializeMesh(mesh_data);
    results.emplace_back(AssetPackagePayloadKind::kMesh, rttr::type::get<Mesh>(),
      std::string{ToUntypedSv(src.stem().u8string())}, std::move(mesh_bytes));
  }

  if (import_materials_) {
    ImportMaterials(mtl_assets, mtl_slots, results);
  }

  if (import_textures_) {
    std::ranges::move(texture_import_results, std::back_inserter(results));
  }

  return true;
}


auto ModelImporter::GetMeshImportSettings() const -> MeshImportSettings const& {
  return mesh_import_settings_;
}


auto ModelImporter::SetMeshImportSettings(MeshImportSettings const& settings) -> void {
  mesh_import_settings_ = settings;
}


auto ModelImporter::GetSubassetImportSettings() const -> std::span<SubassetImportSettings const> {
  return subasset_import_settings_;
}


auto ModelImporter::SetSubassetImportSettings(std::span<SubassetImportSettings const> settings) -> void {
  subasset_import_settings_.assign(settings.begin(), settings.end());
}


auto ModelImporter::IsMaterialImportEnabled() const -> bool {
  return import_materials_;
}


auto ModelImporter::SetMaterialImportEnabled(bool const enabled) -> void {
  // If we're turning off material import, we remove material subasset settings
  if (import_materials_ && !enabled) {
    std::erase_if(subasset_import_settings_, [](SubassetImportSettings const& settings) {
      return settings.kind == SubassetKind::kMaterial;
    });
  }

  // If we're turning on material import, we reset the entire subresouce settings set
  if (!import_materials_ && enabled) {
    subasset_import_settings_.clear();
  }

  import_materials_ = enabled;
}


auto ModelImporter::IsTextureImportEnabled() const -> bool {
  return import_textures_;
}


auto ModelImporter::SetTextureImportEnabled(bool const enabled) -> void {
  // If we're turning off texture import, we remove texture subasset settings
  if (import_textures_ && !enabled) {
    std::erase_if(subasset_import_settings_, [](SubassetImportSettings const& settings) {
      return settings.kind == SubassetKind::kTexture;
    });
  }

  // If we're turning on texture import, we reset the entire subasset settings set
  if (!import_textures_ && enabled) {
    subasset_import_settings_.clear();
  }

  import_textures_ = enabled;
}
}
