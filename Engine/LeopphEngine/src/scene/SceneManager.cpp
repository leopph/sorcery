#include "SceneManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph
{
	SceneManager& SceneManager::Instance()
	{
		static SceneManager instance;
		return instance;
	}


	void SceneManager::AddScene(std::unique_ptr<Scene> scene)
	{
		m_Scenes.emplace_back(std::move(scene));
		SortScenes();
	}


	bool SceneManager::DeleteScene(Scene const* scene)
	{
		if (scene == nullptr)
		{
			return false;
		}

		return DeleteScene(scene->Id());
	}


	bool SceneManager::DeleteScene(std::size_t const id)
	{
		// Always preserve one scene
		if (m_Scenes.size() <= 1)
		{
			return false;
		}

		// Find the scene with the id
		if (auto const it{GetSceneIterator(this, id)};
			it != m_Scenes.end() && it->get()->Id() == id)
		{
			auto const curId{m_Current->Id()};
			m_Scenes.erase(it);
			SortScenes();

			// If we deleted the current scene
			// Set it to the one with the lowest id
			if (curId == id)
			{
				m_Current = m_Scenes.front().get();
			}

			return true;
		}

		return false;
	}


	bool SceneManager::DeleteScene(std::string_view name)
	{
		return DeleteScene(Scene::NameToId(name));
	}


	Scene* SceneManager::FindScene(std::size_t const id) const
	{
		if (auto const it{GetSceneIterator(this, id)};
			it != m_Scenes.end() && it->get()->Id() == id)
		{
			return it->get();
		}

		return nullptr;
	}


	Scene* SceneManager::FindScene(std::string_view const name) const
	{
		return FindScene(Scene::NameToId(name));
	}


	Scene& SceneManager::CurrentScene() const
	{
		return *m_Current;
	}


	void SceneManager::SortScenes()
	{
		std::ranges::sort(m_Scenes, [](auto const& left, auto const& right)
		{
			return left->Id() < right->Id();
		});
	}
}
