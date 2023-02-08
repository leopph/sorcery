#pragma once

#include "Scene.hpp"

#include <memory>
#include <vector>


namespace leopph {
class SceneManager {
	std::vector<std::shared_ptr<Scene>> mScenes;
	std::size_t mActiveSceneIndex{ 0 };

public:
	SceneManager() noexcept = default;
	~SceneManager() noexcept = default;

	LEOPPHAPI auto StartUp() -> void;
	LEOPPHAPI auto ShutDown() noexcept -> void;

	[[nodiscard]] LEOPPHAPI Scene* GetActiveScene();
	LEOPPHAPI Scene* CreateScene(std::string mName);
	LEOPPHAPI void DestroyScene(Scene const* scene);
};
}
