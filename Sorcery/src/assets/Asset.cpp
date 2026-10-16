#include "sorcery/assets/Asset.hpp"


namespace sorcery {
auto Asset::GetAssetId() const noexcept -> AssetId const& {
  return asset_id_;
}


auto Asset::SetAssetId(AssetId const& asset_id) -> void {
  asset_id_ = asset_id;
}
}
