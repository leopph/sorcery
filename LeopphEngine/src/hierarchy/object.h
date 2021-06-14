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

/* Instantiation of std::unordered_map
 * requires std::hash specialization
 * to be visible
 */
#include "../util/modelhash.h"

namespace leopph
{
	namespace impl
	{
		class InstanceData;
	}



	// CLASS REPRESENTING A GAME OBJECT
	// TODO document
	class Object final
	{
	public:
		// instance management
		LEOPPHAPI static Object* Create();
		LEOPPHAPI static void Destroy(Object*& object);
		LEOPPHAPI static Object* Find(const std::string& name);

		LEOPPHAPI Transform& Transform();
		LEOPPHAPI const leopph::Transform& Transform() const;

		LEOPPHAPI const std::string& Name() const;
		LEOPPHAPI void Name(const std::string& newName);

		LEOPPHAPI const std::unordered_set<Model>& Models() const;
		LEOPPHAPI const Model* AddModel(std::filesystem::path path);
		LEOPPHAPI void RemoveModel(const Model*& model);

		LEOPPHAPI const std::set<Component*>& Components() const;

		template<std::derived_from<Component> T>
		T* AddComponent()
		{
			auto component = reinterpret_cast<T*>(*m_Components.emplace(new T{}).first);
			component->m_Object = this;
			component->Init();
			return component;
		}

		LEOPPHAPI void RemoveComponent(Component* behavior);

		template<std::derived_from<Component> T>
		T* GetComponent() const
		{
			for (const auto& x : m_Components)
				if (auto ret = dynamic_cast<T* const>(x); ret != nullptr)
					return const_cast<T*>(ret);

			return nullptr;
		}


	private:
		Object();
		~Object();

		leopph::Transform m_Transform;
		std::string m_Name;

		std::unordered_set<Model> m_Models;
		std::set<Component*> m_Components;

		friend class impl::InstanceData;
	};
}