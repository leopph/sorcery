#pragma once

namespace sorcery {
template<typename T>
struct Extent1D {
  T width;
};


template<typename T>
struct Extent2D {
  T width;
  T height;
};


template<typename T>
struct Extent3D {
  T width;
  T height;
  T depth;
};
}
