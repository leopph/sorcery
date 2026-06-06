#pragma once

#include "external_resource.hpp"
#include "Reflection.hpp"

#include <optional>


namespace sorcery::mage {
struct ResourceDesc {
  // The C++ type of the resource.
  rttr::type type;
  // The category of the resource, if it is an external resource. If not, this will be nullopt.
  std::optional<ExternalResourceCategory> category;
};
}
