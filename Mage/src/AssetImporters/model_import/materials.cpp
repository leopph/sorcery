#include "materials.hpp"

#include <ranges>

#include <assimp/GltfMaterial.h>
#include <spdlog/spdlog.h>

#include "sorcery/host/assets/material_import.hpp"


namespace sorcery::mage::model_import {
namespace {
constexpr auto kBlendFallbackAlphaThreshold = 0.1f;


// Store the texture or return the index of the existing one if it is already stored.
auto TryEmplaceTexture(std::vector<SourceTexture>& src_texes, aiTexture const& tex,
                       aiTextureType const type) -> std::size_t {
  // If type is unknown, be only place it if we don't already have it.
  // Otherwise we just return the first one we find, since we don't care about the type.
  if (type == aiTextureType_UNKNOWN) {
    if (auto const it = std::ranges::find(src_texes, MakeObserver(&tex), &SourceTexture::tex);
      it != src_texes.end()) {
      return std::distance(src_texes.begin(), it);
    }

    src_texes.emplace_back(MakeObserver(&tex), type);
    return src_texes.size() - 1;
  }

  // If type is concrete, we only look for exact matches, and if that fails, then we store.
  if (auto const it{std::ranges::find(src_texes, SourceTexture{.tex = MakeObserver(&tex), .type = type})};
    it != src_texes.end()) {
    return std::distance(src_texes.begin(), it);
  }

  src_texes.emplace_back(MakeObserver(&tex), type);
  return src_texes.size() - 1;
}


// Texture map ids of the returned material hold raw tex_infos indices. They must be fixed up later.
[[nodiscard]]
auto ExtractMaterial(aiScene const& scene, aiMaterial const& mtl, bool const import_textures,
                     std::vector<SourceTexture>& src_texes) -> MaterialAssetData {
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

    auto const register_tex = [&src_texes]
    (aiTexture const& tex, aiTextureType const type, AssetId& id_to_set) {
      // We store the textures' array index now as their package index.
      // Later we will patch this when we're creating the final subasset order in the package.
      auto const arr_idx = TryEmplaceTexture(src_texes, tex, type);
      id_to_set = AssetId{Guid::Invalid(), arr_idx};
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
}


auto IsSrgb(aiTextureType const tex_type) -> bool {
  return
    tex_type == aiTextureType_BASE_COLOR ||
    tex_type == aiTextureType_DIFFUSE ||
    tex_type == aiTextureType_EMISSIVE ||
    tex_type == aiTextureType_EMISSION_COLOR;
}


auto CollectMaterials(aiScene const& scene, bool const import_materials,
                      bool const import_textures) -> CollectedMaterials {
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
      ret.assets[i] = ExtractMaterial(scene, mtl, import_textures, ret.textures);
    }
  }

  return ret;
}


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
      if (auto const idx = id.GetIdInFile()) {
        tex_coverage_thresholds[*idx] = std::max(tex_coverage_thresholds[*idx], tex_threshold);
      }
    };

    set_threshold(mtl.base_color_map);
    set_threshold(mtl.opacity_map);
  }

  return tex_coverage_thresholds;
}


auto AppendUnreferencedTextures(aiScene const& scene, std::vector<SourceTexture>& src_texes) -> void {
  for (std::size_t i{0}; i < scene.mNumTextures; ++i) {
    TryEmplaceTexture(src_texes, *scene.mTextures[i], aiTextureType_UNKNOWN);
  }
}


auto ResolveTextureIds(std::span<MaterialAssetData> const material_data, SubassetIds const& ids) -> void {
  auto const fix_up_tex_id = [&ids](AssetId& id) {
    if (auto const id_in_file = id.GetIdInFile()) {
      id = AssetId{id.GetGuid(), ids.GetTextureId(*id_in_file)};
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


auto AssignMeshMaterialSlots(std::span<MeshData> const imported_meshes,
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


auto BuildTextureResults(
  std::span<SourceTexture const> const src_texes,
  std::span<std::string const> const tex_names,
  std::span<float const> const tex_coverage_thresholds,
  std::span<TextureImportSettings const> const tex_import_settings,
  SubassetIds const& subasset_ids
) -> std::optional<std::vector<AssetImportResult>> {
  std::vector<AssetImportResult> results;

  for (auto const [idx, tex_info] : src_texes | std::views::enumerate) {
    // The texture is compressed
    if (tex_info.tex->mHeight == 0) {
      auto const ext{std::u8string{u8'.'} += reinterpret_cast<char8_t const*>(&tex_info.tex->achFormatHint)};

      TextureImportSource const import_src{
        .file_bytes = std::span{reinterpret_cast<std::byte const*>(tex_info.tex->pcData), tex_info.tex->mWidth},
        .ext = ext
      };

      auto const& saved_settings = tex_import_settings[idx];

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

      auto tex_result = ImportTexture(import_src, import_settings);

      if (!tex_result) {
        return std::nullopt;
      }

      results.emplace_back(subasset_ids.GetTextureId(idx), GetCurrentPayloadVersion(tex_result->payload_kind),
        tex_result->payload_kind, tex_result->runtime_type, tex_names[idx], std::move(tex_result->bytes));
    } else {
      // TODO implement this path
      assert("Found uncompressed embedded texture." && false);
      return std::nullopt;
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

  return results;
}


auto BuildMaterialResults(
  std::span<MaterialAssetData const> const material_data,
  std::span<std::string const> const material_names,
  SubassetIds const& ids
) -> std::vector<AssetImportResult> {
  std::vector<AssetImportResult> out_results;
  out_results.reserve(material_data.size());

  for (auto const [mtl_idx, mtl_data] : std::views::enumerate(material_data)) {
    auto mtl_result = ImportMaterial(mtl_data, {});
    out_results.emplace_back(ids.GetMaterialId(mtl_idx), GetCurrentPayloadVersion(mtl_result.payload_kind),
      mtl_result.payload_kind, mtl_result.runtime_type, material_names[mtl_idx], std::move(mtl_result.bytes));
  }

  return out_results;
}
}
