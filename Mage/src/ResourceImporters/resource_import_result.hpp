#pragma once

#include <cstddef>
#include <vector>

#include "../resource_desc.hpp"


namespace sorcery::mage {
struct ResourceImportResult {
  ResourceDesc desc;
  std::vector<std::byte> bytes;
};
}
