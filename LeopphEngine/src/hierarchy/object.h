#pragma once

#include "../api/leopphapi.h"
#include "../rendering/model.h"
#include "../components/behavior.h"
#include "transform.h"
#include <string>
#include <set>
#include <unordered_set>
#include <concepts>
#include <filesystem>

/* -----------------------------------
 * Instantiation of std::unordered_set
 * requires std::hash specialization
 * to be visible
----------------------------------- */
#include "../util/modelhash.h"

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
		/* Factory to create Objects */
		LEOPPHAPI static Object* Create();
		
		/* Manually destroy an Object.
		 * The given pointer will be set to NULL, and all attached components will be destroyed.
		 * Objects not destroyed manually will be automatically cleaned up upon exiting your application. */
		LEOPPHAPI static void Destroy(Object*& object);

		/* Returns a pointer to the Object that's name is equal to the given string.
		 * Returns NULL if no such Object exists. */
		LEOPPHAPI static Object* Find(const std::string& name);

		/* The Object's Transform describes its spatial properties. */
		LEOPPHAPI Transform& Transform();
		LEOPPHAPI const leopph::Transform& Transform() const;

		/* The Object's name is a unique identifier. */
		LEOPPHAPI const std::string& Name() const;
		LEOPPHAPI void Name(const std::string& newName);

		/* The set of Models attached to the Object. */
		LEOPPHAPI const std::unordered_set<Model>& Models() const;

		/* Attach a new Model to the Object.
		 * Multiple Models can be attached to an Object at the same time. */
		LEOPPHAPI const Model* AddModel(std::filesystem::path path);

		/* Detach and destroy the Model.
		 * If the given Model is attached to the Object, the given pointer will be set to NULL.
		 * Otherwise, the error is silently ignored. */
		LEOPPHAPI void RemoveModel(const Model*& model);

		/* The set of Components attached to the Object. */
		LEOPPHAPI const std::set<Component*>& Components() const;

		/* Attach a new Component of type T to the Object.
		 * It is constructed in-place, and a pointer to it is returned. */
		template<std::derived_from<Component> T>
		T* AddComponent()
		{
			auto component = new T{};
			component->SetOwnership(this);
			component->Init();
			return component;
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


	private:
		Object();

		leopph::Transform m_Transform;
		std::string m_Name;
		std::unordered_set<Model> m_Models;

		friend class impl::InstanceHolder;
	};
}