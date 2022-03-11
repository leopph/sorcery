#pragma once

#include <cstddef>
#include <Leopph.hpp>
#include <optional>
#include <unordered_map>
#include <vector>


namespace demo
{
	class SceneSwitcher
	{
		private:
			using IdType = std::size_t;
			using SceneDataType = std::vector<leopph::Entity*>;

			std::optional<IdType> m_Active;
			std::unordered_map<IdType, SceneDataType> m_Scenes;

			[[nodiscard]] static
			auto GenerateId() noexcept -> IdType;

		public:
			class Scene
			{
				friend class SceneSwitcher;

				public:
					auto Add(leopph::Entity* entity) const -> void;
					[[nodiscard]]
					auto Id() const -> IdType;

				private:
					using PointerType = decltype(m_Scenes)::value_type*;

					explicit Scene(PointerType pointer);

					PointerType m_Pointer;
			};


			// Create a Scene.
			// The returned object is a proxy to internally stored data.
			[[nodiscard]]
			auto CreateScene() -> Scene;

			// Deactivates all components in the active scene if any,
			// then activates all components in the scene with the passed id.
			auto ActivateScene(std::size_t id) -> void;
			// Deactivates all components in the active scene if any,
			// then activates all components in the scene with the passed id.
			auto ActivateScene(Scene scene) -> void;

			// Returns a Scene object representing the currently active scene.
			// Returns an empty optional if no scene is active.
			[[nodiscard]]
			auto ActiveScene() -> std::optional<Scene>;
	};
}
