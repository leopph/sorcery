#include "SceneManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph
{
	auto SceneManager::Instance() -> SceneManager&
	{
		static SceneManager instance;
		return instance;
	}


	auto SceneManager::AddScene(std::unique_ptr<Scene> scene) -> void
	{
		m_Scenes.emplace_back(std::move(scene));
		SortScenes();
	}


	auto SceneManager::DeleteScene(const Scene* scene) -> bool
	{
		if (scene == nullptr)
		{
			return false;
		}

		return DeleteScene(scene->Id());
	}


	auto SceneManager::DeleteScene(const std::size_t id) -> bool
	{
		// Always preserve one scene
		if (m_Scenes.size() <= 1)
		{
			return false;
		}

		// Find the scene with the id
		if (const auto it{GetSceneIterator(this, id)};
			it != m_Scenes.end() && it->get()->Id() == id)
		{
			const auto curId{m_Current->Id()};
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


	auto SceneManager::DeleteScene(std::string_view name) -> bool
	{
		return DeleteScene(Scene::NameToId(name));
	}


	auto SceneManager::FindScene(const std::size_t id) const -> Scene*
	{
		if (const auto it{GetSceneIterator(this, id)};
			it != m_Scenes.end() && it->get()->Id() == id)
		{
			return it->get();
		}

		return nullptr;
	}


	auto SceneManager::FindScene(const std::string_view name) const -> Scene*
	{
		return FindScene(Scene::NameToId(name));
	}


	auto SceneManager::CurrentScene() const -> Scene&
	{
		return *m_Current;
	}


	auto SceneManager::SortScenes() -> void
	{
		std::ranges::sort(m_Scenes, [](const auto& left, const auto& right)
		{
			return left->Id() < right->Id();
		});
	}
}
