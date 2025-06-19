#pragma once

#include "extent.hpp"
#include "point.hpp"


namespace sorcery {
struct Viewport {
  Point2D<unsigned> position;
  Extent2D<unsigned> extent;
};


struct NormalizedViewport {
  float left;
  float top;
  float right;
  float bottom;
};
}


#include "viewport.inl"
