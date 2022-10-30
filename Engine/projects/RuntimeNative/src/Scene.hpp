#pragma once

#include "Entity.hpp"

#include <string>
#include <vector>


namespace leopph
{
	class Scene
	{
		std::string mName;
		std::vector<Entity*> mEntities;

	public:
		Scene(std::string name);

		void AddEntity(Entity* entity);
		void RemoveEntity(Entity* entity);
	};
}