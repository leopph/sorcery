#pragma once

#include "leopphapi.h"
#include "model.h"
#include "behavior.h"
#include "vector.h"
#include "quaternion.h"
#include "transform.h"


#include <vector>
#include <string>
#include <set>
#include <concepts>



namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)



	namespace implementation
	{
		class InstanceData;
	}



	// CLASS REPRESENTING A GAME OBJECT
	class LEOPPHAPI Object final
	{
	public:
		friend class implementation::InstanceData;


		// instance management
		static Object* Create();
		static void Destroy(Object*& object);
		static Object* Find(const std::string& name);




		leopph::Transform& Transform();
		const leopph::Transform& Transform() const;



		const std::string& Name() const;
		void Name(std::string newName);




		const std::vector<Model>& Models() const;
		void AddModel(Model&& model);
		void RemoveModel(size_t index);




		const std::set<Component*>& Components() const;

		template<std::derived_from<Component> T>
		T* AddComponent()
		{
			return reinterpret_cast<T*>(*m_Components.emplace(new T{ *this }).first);
		}

		void RemoveComponent(Behavior* behavior);

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




#pragma warning(pop)
}