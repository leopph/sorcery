#pragma once

#include <vector>


namespace sorcery {
struct StaticMeshSubmeshIndex {
  int staticMeshIdx;
  int submeshIdx;
};


struct Visibility {
  std::pmr::vector<int> lightIndices;
  std::pmr::vector<StaticMeshSubmeshIndex> staticMeshIndices;
};
}
