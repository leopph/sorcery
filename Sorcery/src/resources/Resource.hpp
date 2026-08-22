#pragma once

#include "../Object.hpp"
#include "../resource_id.hpp"


namespace sorcery {
class Resource : public Object {
  RTTR_ENABLE(Object)

public:
  [[nodiscard]] LEOPPHAPI auto GetResId() const noexcept -> ResourceId const&;
  LEOPPHAPI auto SetResId(ResourceId const& res_id) -> void;

private:
  ResourceId res_id_{Guid::Generate(), 0};
};
}
