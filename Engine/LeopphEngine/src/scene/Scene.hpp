#pragma once

#include "Entity.hpp"

#include <string>
#include <vector>


namespace leopph
{
	class Scene
	{
		friend class Entity;
		friend class SceneManager;

		public:
			[[nodiscard]] std::span<Entity* const> get_entities() const;


		private:
			explicit Scene(std::string name);

		public:
			Scene(Scene const& other) = delete;
			Scene& operator=(Scene const& other) = delete;

			Scene(Scene&& other) = delete;
			Scene& operator=(Scene&& other) = delete;

			~Scene() = default;


		private:
			void register_entity(Entity* entity);
			void unregister_entity(Entity* entity);

			std::vector<Entity*> mEntities;
			std::string mName;
	};
}
