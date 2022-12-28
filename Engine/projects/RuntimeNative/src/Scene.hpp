#pragma once

#include "Entity.hpp"
#include "Object.hpp"

#include <string>
#include <memory>
#include <vector>


namespace leopph
{
	class Scene final : public Object
	{
		friend class SceneManager;

		std::string mName;
		std::vector<std::unique_ptr<Entity>> mEntities;

		explicit Scene(std::string mName);

	public:
		LEOPPHAPI Entity* CreateEntity();
		LEOPPHAPI void DestroyEntity(Entity const* entity);

		[[nodiscard]] auto GetSerializationType() const -> Type override;
	};
}