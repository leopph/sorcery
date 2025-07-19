#pragma once

#include "../Object.hpp"
#include "../resource_id.hpp"


namespace sorcery {
class Resource : public Object {
  RTTR_ENABLE(Object)

public:
  [[nodiscard]] LEOPPHAPI auto GetId() const noexcept -> ResourceId const&;
  LEOPPHAPI auto SetId(ResourceId const& res_id) -> void;

private:
  ResourceId id_{Guid::Generate(), 0};
};
}
