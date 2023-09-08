#pragma once

#include "Core.hpp"
#include "Math.hpp"

#include <cstdint>


namespace sorcery {
struct Color {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  std::uint8_t alpha;

  Color() = default;
  LEOPPHAPI Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
  LEOPPHAPI explicit Color(Vector4 const& vec);
  LEOPPHAPI explicit operator Vector4() const;

  LEOPPHAPI static Color Black();
  LEOPPHAPI static Color Red();
  LEOPPHAPI static Color Green();
  LEOPPHAPI static Color Blue();
  LEOPPHAPI static Color Cyan();
  LEOPPHAPI static Color Magenta();
  LEOPPHAPI static Color Yellow();
  LEOPPHAPI static Color White();
};
}
