#include "SceneElement.hpp"

#include <vector>

namespace leopph {
	namespace {
		std::vector<SceneElement*> gAllSceneElements;
	}


	SceneElement::SceneElement() {
		gAllSceneElements.emplace_back(this);
	}


	SceneElement::~SceneElement() {
		std::erase(gAllSceneElements, this);
	}


	auto GetAllSceneElements() -> std::span<SceneElement* const> {
		return gAllSceneElements;
	}
}