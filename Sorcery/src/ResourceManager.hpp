#pragma once

#include "Core.hpp"
#include "Resource.hpp"


namespace sorcery {
class ResourceManager {
  struct ResourceGuidLess {
    using is_transparent = void;
    [[nodiscard]] auto operator()(std::shared_ptr<Resource> const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool;
    [[nodiscard]] auto operator()(std::shared_ptr<Resource> const& lhs, Guid const& rhs) const noexcept -> bool;
    [[nodiscard]] auto operator()(Guid const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool;
  };


  std::set<std::shared_ptr<Resource>, ResourceGuidLess> mResources;

public:
  LEOPPHAPI auto AddResource(std::shared_ptr<Resource> res) -> std::weak_ptr<Resource>;
  [[nodiscard]] LEOPPHAPI auto FindResource(Guid const& guid) -> std::weak_ptr<Resource>;
};


LEOPPHAPI extern ResourceManager gResourceManager;
}
