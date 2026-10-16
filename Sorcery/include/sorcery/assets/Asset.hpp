#pragma once

#include <sorcery/Object.hpp>
#include <sorcery/asset_id.hpp>


namespace sorcery {
class Asset : public Object {
  RTTR_ENABLE(Object)

public:
  [[nodiscard]] LEOPPHAPI auto GetAssetId() const noexcept -> AssetId const&;
  LEOPPHAPI auto SetAssetId(AssetId const& asset_id) -> void;

private:
  AssetId asset_id_{Guid::Generate(), 0};
};
}
