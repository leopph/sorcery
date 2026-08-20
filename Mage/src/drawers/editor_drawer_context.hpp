#pragma once

#include "observer_ptr.hpp"


namespace sorcery::mage {
class EditorDrawerRegistry;


struct EditorDrawerContext {
  ObserverPtr<EditorDrawerRegistry> registry;
};
}
