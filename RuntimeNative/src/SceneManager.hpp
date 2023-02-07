#pragma once

#include "Scene.hpp"

#include <memory>
#include <vector>


namespace leopph {
class SceneManager {
	static std::vector<std::shared_ptr<Scene>> sScenes;
	static std::size_t sActiveSceneIndex;

public:
	[[nodiscard]] LEOPPHAPI static Scene* GetActiveScene();
	LEOPPHAPI static Scene* CreateScene(std::string mName);
};
}
