#pragma once

#include "Core.hpp"
#include "Math.hpp"


namespace sorcery {
struct Color {
  u8 red;
  u8 green;
  u8 blue;
  u8 alpha;

  Color() = default;
  LEOPPHAPI Color(u8 red, u8 green, u8 blue, u8 alpha);
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
