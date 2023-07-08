#pragma once

#include <vector>


namespace sorcery {
struct Visibility {
  std::vector<int> lightIndices;
  std::vector<int> staticMeshIndices;
};
}
