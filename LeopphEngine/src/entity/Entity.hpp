#pragma once

#include "../api/LeopphApi.hpp"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"

#include <concepts>
#include <unordered_set>
#include <string>
#include <utility>


namespace leopph
{
	namespace impl
	{
		class DataManager;
	}


	/*---------------------------------------------------------------------------------------------------------------
	Entities are the bases of the entity hierarchy. They have spatial characteristics through a Transform,
	can have Models that are later rendered according to these characteristics, and have components attached to them,
	that may give them their own unique properties or behaviors.
	See "component.h", "behavior.h", and "model.h" for additional information.
	---------------------------------------------------------------------------------------------------------------*/
	class Entity final
	{
	public:
		LEOPPHAPI Entity();
		LEOPPHAPI explicit  Entity(std::string name);

		Entity(const Entity&) = delete;
		Entity(Entity&&) = delete;
		void operator=(const Entity&) = delete;
		void operator=(Entity&&) = delete;

		LEOPPHAPI ~Entity();

		/* Returns a pointer to the Entity that's name is equal to the given string.
		 * Returns NULL if no such Entity exists. */
		LEOPPHAPI static Entity* Find(const std::string& name);

		/* The set of Components attached to the Entity. */
		LEOPPHAPI const std::unordered_set<Component*>& Components() const;

		/* Attach a new Component of type T to the Entity.
		 * It is constructed in-place, and a pointer to it is returned. */
		template<std::derived_from<Component> T, class... Args>
		T* AddComponent(Args&&... args)
		{
			return new T{ *this, std::forward<Args>(args)... };
		}

		/* Remove the given Component from the Entity.
		 * The component is detached and destroyed. */
		LEOPPHAPI void RemoveComponent(Component* behavior);

		/* Look for a Component of type T that is attached to the Entity.
		 * If there is none, NULL is returned.
		 * Otherwise, a pointer to the first matching Component is returned.
		 * There are no guarantees of the order of Components attached to the Entity. */
		template<std::derived_from<Component> T>
		T* GetComponent() const
		{
			for (const auto& x : Components())
				if (auto ret = dynamic_cast<T* const>(x); ret != nullptr)
					return const_cast<T*>(ret);

			return nullptr;
		}

		/* The Entity's name is a unique identifier. */
		const std::string& Name;
		/* The Entity's Transform describes its spatial properties. */
		Transform* const& Transform;


	private:
		std::string m_Name;
		leopph::Transform* m_Transform;
	};
}