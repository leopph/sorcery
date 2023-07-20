#pragma once

#include "../Object.hpp"
#include "../Guid.hpp"


namespace sorcery {
class Resource : public Object {
  RTTR_ENABLE(Object)
  Guid mGuid;

public:
  [[nodiscard]] LEOPPHAPI auto GetGuid() const -> Guid const&;
  LEOPPHAPI auto SetGuid(Guid const& guid) -> void;
};
}
