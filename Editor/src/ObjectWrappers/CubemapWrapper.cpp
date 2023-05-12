#include "ObjectWrappers.hpp"

#include "../CubemapImporter.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Cubemap>::GetImporter() -> Importer& {
  CubemapImporter static cubemapImporter;
  return cubemapImporter;
}
}
