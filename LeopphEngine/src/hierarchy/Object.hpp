#pragma once

#include "../api/leopphapi.h"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"
#include "../util/ObjectProperties.hpp"

#include <concepts>
#include <set>
#include <string>
#include <utility>

namespace leopph
{
	namespace impl
	{
		class InstanceHolder;
	}


	/*---------------------------------------------------------------------------------------------------------------
	Objects are the bases of the entity hierarchy. They have spatial characteristics through a Transform,
	can have Models that are later rendered according to these characteristics, and have components attached to them,
	that may give them their own unique properties or behaviors.
	See "component.h", "behavior.h", and "model.h" for additional information.
	---------------------------------------------------------------------------------------------------------------*/
	class Object final
	{
	public:
		LEOPPHAPI explicit Object(const ObjectProperties& properties);
		LEOPPHAPI explicit Object(bool isStatic, std::string name);
		LEOPPHAPI explicit Object(bool isStatic);
		LEOPPHAPI explicit  Object(std::string name);
		LEOPPHAPI Object();
		LEOPPHAPI ~Object();

		Object(const Object&) = delete;
		Object(Object&&) = delete;
		void operator=(const Object&) = delete;
		void operator=(Object&&) = delete;

		/* Returns a pointer to the Object that's name is equal to the given string.
		 * Returns NULL if no such Object exists. */
		LEOPPHAPI static Object* Find(const std::string& name);

		/* The Object's Transform describes its spatial properties. */
		LEOPPHAPI Transform& Transform();
		[[nodiscard]] LEOPPHAPI const leopph::Transform& Transform() const;

		/* The set of Components attached to the Object. */
		LEOPPHAPI const std::set<Component*>& Components() const;

		/* Attach a new Component of type T to the Object.
		 * It is constructed in-place, and a pointer to it is returned. */
		template<std::derived_from<Component> T, class... Args>
		T* AddComponent(Args&&... args)
		{
			return new T{ *this, std::forward<Args>(args)... };
		}

		/* Remove the given Component from the Object.
		 * If it is not attached to this Object, std::invalid_exception is thrown.
		 * Otherwise, the component is detached and destroyed. */
		LEOPPHAPI void RemoveComponent(Component* behavior);

		/* Look for a Component of type T that is attached to the Object.
		 * If there is none, NULL is returned.
		 * Otherwise, a pointer to the first matching Component is returned.
		 * There are no guarantees of the order of Components attached to the Object. */
		template<std::derived_from<Component> T>
		T* GetComponent() const
		{
			for (const auto& x : Components())
				if (auto ret = dynamic_cast<T* const>(x); ret != nullptr)
					return const_cast<T*>(ret);

			return nullptr;
		}

		/* Static objects cannot be moved, rotated, or scaled. */
		const bool isStatic;
		/* The Object's name is a unique identifier. */
		const std::string& name;

	private:
		std::string m_Name;
		mutable leopph::Transform* m_Transform;
	};
}