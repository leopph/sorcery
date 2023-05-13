#include "ObjectWrappers.hpp"

#include "../AssetLoaders/CubemapLoader.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Cubemap>::GetLoader() -> AssetLoader& {
  CubemapLoader static loader;
  return loader;
}
}
