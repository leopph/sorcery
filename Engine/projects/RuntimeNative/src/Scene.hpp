#pragma once

#include "Entity.hpp"

#include <string>
#include <memory>
#include <vector>


namespace leopph
{
	class Scene
	{
		friend class SceneManager;

	private:
		std::string mName;
		std::vector<std::unique_ptr<Entity>> mEntities;

		Scene(std::string mName);

	public:
		LEOPPHAPI Entity* CreateEntity();
		LEOPPHAPI void DestroyEntity(Entity const* entity);
	};
}