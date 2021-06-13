#pragma once

#include "../api/leopphapi.h"
#include "../rendering/model.h"
#include "../components/behavior.h"
#include "transform.h"


#include <vector>
#include <string>
#include <set>
#include <concepts>



namespace leopph
{
	namespace impl
	{
		class InstanceData;
	}



	// CLASS REPRESENTING A GAME OBJECT
	class Object final
	{
	public:
		friend class impl::InstanceData;


		// instance management
		LEOPPHAPI static Object* Create();
		LEOPPHAPI static void Destroy(Object*& object);
		LEOPPHAPI static Object* Find(const std::string& name);




		LEOPPHAPI leopph::Transform& Transform();
		LEOPPHAPI const leopph::Transform& Transform() const;



		LEOPPHAPI const std::string& Name() const;
		LEOPPHAPI void Name(std::string newName);




		LEOPPHAPI const std::vector<Model>& Models() const;
		LEOPPHAPI void AddModel(Model&& model);
		LEOPPHAPI void RemoveModel(size_t index);




		LEOPPHAPI const std::set<Component*>& Components() const;

		template<std::derived_from<Component> T>
		T* AddComponent()
		{
			return reinterpret_cast<T*>(*m_Components.emplace(new T{ *this }).first);
		}

		LEOPPHAPI void RemoveComponent(Behavior* behavior);

		template<std::derived_from<Component> T>
		T* GetComponent() const
		{
			// TODO
		}


	private:
		Object();
		~Object();

		leopph::Transform m_Transform;
		std::string m_Name;

		std::vector<Model> m_Models;
		std::set<Component*> m_Components;
	};
}