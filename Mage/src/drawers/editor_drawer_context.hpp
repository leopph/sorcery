#pragma once

#include "observer_ptr.hpp"


namespace sorcery::mage {
class EditorDrawerRegistry;
class ImGuiTextureReferences;


struct EditorDrawerContext {
  ObserverPtr<EditorDrawerRegistry> registry;
  ObserverPtr<ImGuiTextureReferences> tex_refs;
};
}
