#pragma once

#include <sorcery/extent.hpp>
#include <sorcery/point.hpp>


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


#include <sorcery/viewport.inl>
