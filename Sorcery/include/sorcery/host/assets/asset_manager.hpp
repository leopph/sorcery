#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/mutex.hpp>
#include <sorcery/object_ptr.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/asset_ref.hpp>
#include <sorcery/assets/Material.hpp>
#include <sorcery/assets/Mesh.hpp>
#include <sorcery/assets/Asset.hpp>


namespace sorcery {
class JobSystem;
struct Job;


class AssetManager {
public:
  struct AssetDescription {
    std::string name;
    rttr::type type;
  };


  struct AssetInfo {
    AssetId id;
    std::string name;
    rttr::type type;
  };


  SORCERYAPI explicit AssetManager(JobSystem& job_system);

  template<std::derived_from<Asset> AssetType = Asset>
  auto Resolve(AssetId const& asset_id) -> ObjectPtr<AssetType>;

  SORCERYAPI
  auto Unload(AssetId const& asset_id) -> void;

  SORCERYAPI
  auto UnloadAll() -> void;

  [[nodiscard]] SORCERYAPI
  auto IsLoaded(AssetId const& asset_id) -> bool;

  template<std::derived_from<Asset> AssetType>
  auto Add(std::unique_ptr<AssetType> asset) -> AssetRef<AssetType>;

  template<std::derived_from<Asset> AssetType = Asset>
  [[nodiscard]]
  auto Remove(AssetId const& asset_id) -> std::unique_ptr<AssetType>;

  SORCERYAPI
  auto UpdateMappings(std::map<AssetId, AssetDescription> asset_mappings,
                      std::map<Guid, std::filesystem::path> file_mappings) -> void;

  template<std::derived_from<Asset> T>
  auto GetInfoForAssetsOfType(std::vector<AssetInfo>& out) -> void;

  SORCERYAPI
  auto GetInfoForAssetsOfType(rttr::type const& type, std::vector<AssetInfo>& out) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetDefaultMaterial() const noexcept -> AssetRef<Material>;

  [[nodiscard]] SORCERYAPI
  auto GetCubeMesh() const noexcept -> AssetRef<Mesh>;

  [[nodiscard]] SORCERYAPI
  auto GetPlaneMesh() const noexcept -> AssetRef<Mesh>;

  [[nodiscard]] SORCERYAPI
  auto GetSphereMesh() const noexcept -> AssetRef<Mesh>;

  // Called only internally!
  auto CreateDefaultAssets() -> void;

  constexpr static std::string_view EXTERNAL_ASSET_EXT{".bin"};
  constexpr static std::string_view SCENE_ASSET_EXT{".scene"};
  constexpr static std::string_view MATERIAL_ASSET_EXT{".mtl"};

private:
  struct AssetIdLess {
    using is_transparent = void;

    [[nodiscard]] SORCERYAPI auto operator()(std::unique_ptr<Asset> const& lhs,
                                             std::unique_ptr<Asset> const& rhs) const noexcept -> bool;

    [[nodiscard]] SORCERYAPI auto operator()(std::unique_ptr<Asset> const& lhs,
                                             AssetId const& rhs) const noexcept -> bool;

    [[nodiscard]] SORCERYAPI auto operator()(AssetId const& lhs,
                                             std::unique_ptr<Asset> const& rhs) const noexcept -> bool;
  };


  [[nodiscard]] SORCERYAPI auto InternalLoadAsset(AssetId const& asset_id,
                                                     AssetDescription const& desc) -> ObjectPtr<Asset>;
  [[nodiscard]] static
  auto LoadTexture(std::span<std::byte const> bytes) -> std::unique_ptr<Asset>;

  [[nodiscard]] static
  auto LoadMesh(std::span<std::byte const> bytes) -> std::unique_ptr<Asset>;

  [[nodiscard]] static
  auto LoadMaterial(std::span<std::byte const> bytes,
                    YamlDeserializeContext const& ctx) -> std::unique_ptr<Asset>;

  [[nodiscard]] static
  auto LoadPrefab(std::span<std::byte const> bytes,
                  YamlDeserializeContext const& ctx) -> std::unique_ptr<Asset>;

  ObserverPtr<JobSystem> job_system_;

  Mutex<std::set<std::unique_ptr<Asset>, AssetIdLess>, true> loaded_assets_;

  std::unique_ptr<Material> default_mtl_;
  std::unique_ptr<Mesh> cube_mesh_;
  std::unique_ptr<Mesh> plane_mesh_;
  std::unique_ptr<Mesh> sphere_mesh_;

  // We can safely hold default assets with ObserverPtr
  // because we have owning pointers to them in other members
  std::vector<ObserverPtr<Asset>> default_assets_;

  Mutex<std::map<AssetId, AssetDescription>, true> asset_mappings_;
  Mutex<std::map<Guid, std::filesystem::path>, true> file_mappings_;

  Mutex<std::map<AssetId, ObserverPtr<Job>>, true> loader_jobs_;

  inline static Guid const default_asset_guid_{1, 0};
  inline static AssetId const default_mtl_asset_id_{default_asset_guid_, 0};
  inline static AssetId const cube_mesh_asset_id_{default_asset_guid_, 1};
  inline static AssetId const plane_mesh_asset_id_{default_asset_guid_, 2};
  inline static AssetId const sphere_mesh_asset_id_{default_asset_guid_, 3};
};
}


#include <sorcery/host/assets/asset_manager.inl>
