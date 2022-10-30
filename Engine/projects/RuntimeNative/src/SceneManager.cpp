#include "SceneManager.hpp"

namespace leopph
{
	std::vector<std::unique_ptr<Scene>> SceneManager::sScenes{};
	std::size_t SceneManager::sActiveSceneIndex{ 0 };


	Scene* SceneManager::GetActiveScene()
	{
		if (sActiveSceneIndex >= sScenes.size())
		{
			if (sScenes.empty())
			{
				sActiveSceneIndex = 0;
				return CreateScene("New Scene");
			}

			sActiveSceneIndex = 0;
		}

		return sScenes[sActiveSceneIndex].get();
	}


	Scene* SceneManager::CreateScene(std::string name)
	{
		return sScenes.emplace_back(std::make_unique<Scene>(std::move(name))).get();
	}
}