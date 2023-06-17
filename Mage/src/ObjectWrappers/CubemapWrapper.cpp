#include "ObjectWrappers.hpp"

#include "../AssetLoaders/CubemapLoader.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<Cubemap>::GetLoader() -> AssetLoader& {
  CubemapLoader static loader;
  return loader;
}
}
