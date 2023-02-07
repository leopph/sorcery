#include "SceneManager.hpp"

namespace leopph {
auto SceneManager::StartUp() -> void {}

auto SceneManager::ShutDown() noexcept -> void {
	mScenes.clear();
}

Scene* SceneManager::GetActiveScene() {
	if (mActiveSceneIndex >= mScenes.size()) {
		if (mScenes.empty()) {
			mActiveSceneIndex = 0;
			return CreateScene("New Scene");
		}

		mActiveSceneIndex = 0;
	}

	return mScenes[mActiveSceneIndex].get();
}


Scene* SceneManager::CreateScene(std::string mName) {
	return mScenes.emplace_back(new Scene{ std::move(mName) }).get();
}
}
