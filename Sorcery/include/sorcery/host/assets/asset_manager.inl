#pragma once

#include <utility>

#include <sorcery/util.hpp>


namespace sorcery {
template<std::derived_from<Asset> AssetType>
auto AssetManager::Resolve(AssetId const& asset_id) -> ObjectPtr<AssetType> {
  if (!asset_id.IsValid()) {
    return nullptr;
  }

  // Check default assets
  for (auto const def_asset : default_assets_) {
    if (def_asset->GetAssetId() == asset_id) {
      if constexpr (!std::is_same_v<AssetType, Asset>) {
        return MakeObjectPtr(ReflCast<AssetType>(def_asset));
      } else {
        return MakeObjectPtr(def_asset);
      }
    }
  }

  // Check loaded assets
  {
    auto const assets{loaded_assets_.LockShared()};

    if (auto const it{assets->find(asset_id)}; it != std::end(*assets)) {
      if constexpr (!std::is_same_v<AssetType, Asset>) {
        return MakeObjectPtr(MakeObserver(ReflCast<AssetType*>(it->get())));
      } else {
        return MakeObjectPtr(MakeObserver(it->get()));
      }
    }
  }

  // Check in file mappings

  std::optional<AssetDescription> desc;

  {
    auto const mappings{asset_mappings_.LockShared()};
    if (auto const it{mappings->find(asset_id)}; it != std::end(*mappings)) {
      desc = it->second;
    }
  }

  // Load asset

  if (desc) {
    if (auto const asset{InternalLoadAsset(asset_id, *desc)}) {
      if constexpr (!std::is_same_v<AssetType, Asset>) {
        return ReflCast<AssetType>(asset);
      } else {
        return asset;
      }
    }
  }

  return nullptr;
}


template<std::derived_from<Asset> AssetType>
auto AssetManager::Add(std::unique_ptr<AssetType> asset) -> AssetRef<AssetType> {
  if (asset && asset->GetAssetId().IsValid()) {
    return MakeAssetRef(
      MakeObserver(static_cast<AssetType*>(loaded_assets_.Lock()->emplace(std::move(asset)).first->get())));
  }

  return nullptr;
}


template<std::derived_from<Asset> AssetType>
auto AssetManager::Remove(AssetId const& asset_id) -> std::unique_ptr<AssetType> {
  auto assets{loaded_assets_.Lock()};

  if (auto const it{
    std::ranges::find_if(*assets, [&asset_id](std::unique_ptr<Asset> const& asset) {
      if constexpr (std::is_same_v<AssetType, Asset>) {
        return asset->GetAssetId() == asset_id;
      } else {
        return asset->GetAssetId() == asset_id && rttr::type::get(*asset).get_raw_type().is_derived_from<AssetType>();
      }
    })
  }; it != std::end(*assets)) {
    auto node{assets->extract(it)};
    return static_unique_ptr_cast<AssetType>(std::move(node.value()));
  }

  return nullptr;
}


template<std::derived_from<Asset> T>
auto AssetManager::GetInfoForAssetsOfType(std::vector<AssetInfo>& out) -> void {
  GetInfoForAssetsOfType(rttr::type::get<T>(), out);
}
}
