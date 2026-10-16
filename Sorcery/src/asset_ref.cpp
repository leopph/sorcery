#include "sorcery/asset_ref.hpp"

#include "sorcery/host/app.hpp"
#include "sorcery/host/assets/asset_manager.hpp"


namespace sorcery::detail {
auto ResolveAsset(AssetId const& id) -> ObjectPtr<Asset> {
  return App::Instance().GetAssetManager().Resolve(id);
}
}
