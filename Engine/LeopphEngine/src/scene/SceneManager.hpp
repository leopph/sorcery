#pragma once

#include "LeopphApi.hpp"
#include "Scene.hpp"

#include <memory>
#include <string_view>
#include <vector>


namespace leopph
{
	class SceneManager
	{
		public:
			[[nodiscard]] LEOPPHAPI static SceneManager& Instance();

			void AddScene(std::unique_ptr<Scene> scene);

			// Deletes the specified scene instance.
			// Nullptr is silently ignored.
			// If it is the last scene instance, it will not be deleted.
			// Returns whether deletion took place.
			// If the active scene is deleted, the new active scene will be the one with the lowest id.
			LEOPPHAPI bool DeleteScene(Scene const* scene);

			// Deletes the scene with the specified id.
			// Unused ids are silently ignored.
			// If there is only one scene instance, it will not be deleted.
			// Returns whether deletion took place.
			// If the active scene is deleted, the new active scene will be the one with the lowest id.
			LEOPPHAPI bool DeleteScene(std::size_t id);

			// Deletes the scene that was created with the specified name.
			// Unused names are silently ignored.
			// If there is only one scene instance, it will not be deleted.
			// Returns whether deletion took place.
			// If the active scene is deleted, the new active scene will be the one with the lowest id.
			LEOPPHAPI bool DeleteScene(std::string_view name);

			// Returns a pointer to the scene with the specified id,
			// Or nullptr if not found.
			[[nodiscard]] LEOPPHAPI Scene* FindScene(std::size_t id) const;

			// Returns a pointer to the scene that was created with the specified name,
			// Or nullptr if not found.
			[[nodiscard]] LEOPPHAPI Scene* FindScene(std::string_view name) const;

			// Returns the currently active scene.
			// There is always one active scene.
			[[nodiscard]] LEOPPHAPI Scene& CurrentScene() const;

			SceneManager(SceneManager const& other) = delete;
			SceneManager& operator=(SceneManager const& other) = delete;

			SceneManager(SceneManager&& other) noexcept = delete;
			SceneManager& operator=(SceneManager&& other) noexcept = delete;

		private:
			SceneManager() = default;
			~SceneManager() = default;

			// Returns an iterator to the first scene that's id is not less than the passed id.
			// Returns end iterator if there is none.
			[[nodiscard]] constexpr static auto GetSceneIterator(auto self, std::size_t id);

			// Sorts the scenes in ascending order based on id.
			void SortScenes();

			Scene* m_Current{new Scene{0}};
			std::vector<std::unique_ptr<Scene>> m_Scenes{
				[this]
				{
					std::vector<std::unique_ptr<Scene>> scenes;
					scenes.emplace_back(m_Current);
					return scenes;
				}()
			};
	};


	constexpr auto SceneManager::GetSceneIterator(auto self, std::size_t const id)
	{
		return std::lower_bound(self->m_Scenes.begin(), self->m_Scenes.end(), id, [](std::unique_ptr<Scene> const& scene, std::size_t const targetId)
		{
			return scene->Id() < targetId;
		});
	}
}
