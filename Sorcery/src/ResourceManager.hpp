#pragma once

#include "Core.hpp"
#include "Object.hpp"

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cstdint>
#include <concepts>


namespace sorcery {
class ResourceManager {
  struct ResourceEntry {
    std::unique_ptr<Object> resource;
    std::uint32_t version;
  };


  std::unordered_map<std::type_index, std::vector<ResourceEntry>> mResources;
};


template<std::derived_from<Object> T>
class ResourceHandle {
  friend class ResourceManager;
  std::uint32_t mIdx;
  std::uint32_t mVersion;

public:
  [[nodiscard]] bool isValid() const noexcept { return mIdx != 0; }
};


LEOPPHAPI extern ResourceManager gResourceManager;
}
