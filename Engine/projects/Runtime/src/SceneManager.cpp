#include "SceneManager.hpp"

#include <utility>


namespace leopph
{
	Scene& SceneManager::create_scene(std::string name)
	{
		mScenes.emplace_back(new Scene{std::move(name)});
		return *mScenes.back();
	}



	Scene& SceneManager::get_active_scene() const
	{
		return *mActiveScene;
	}



	void SceneManager::set_active_scene(Scene& scene)
	{
		mActiveScene = &scene;
	}



	SceneManager::SceneManager()
	{
		mActiveScene = &create_scene();
	}
}
