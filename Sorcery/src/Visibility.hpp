#pragma once

#include <vector>


namespace sorcery {
struct Visibility {
  std::pmr::vector<int> lightIndices;
  std::pmr::vector<int> staticMeshIndices;
};
}
