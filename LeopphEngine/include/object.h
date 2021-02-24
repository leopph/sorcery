#pragma once

#include "leopphapi.h"
#include "model.h"
#include "behavior.h"
#include "vector.h"
#include "quaternion.h"


#include <vector>
#include <string>
#include <set>



namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS REPRESENTING A GAME OBJECT
	class LEOPPHAPI Object final
	{
	public:
		static Object* Create();
		static void Destroy(Object*& object);
		static Object* Find(const std::string& name);

		static void UpdateAll();

		const std::vector<Model>& Models() const;
		void AddModel(Model&& model);
		void RemoveModel(size_t index);

		const std::set<Behavior*>& Behaviors() const;

		template<class T>
		T* AddBehavior()
		{
			return reinterpret_cast<T*>(*m_Behaviors.emplace(new T{ *this }).first);
		}

		void RemoveBehavior(Behavior* behavior);
		template<class T>
		T* GetBehavior() const
		{
			// TODO
		}

		const std::string& Name() const;
		void Name(std::string newName);

		const Vector3& Position() const;
		void Position(Vector3 newPos);

		const Quaternion& Rotation() const;
		void Rotation(Quaternion newRot);

		const Vector3& Scale() const;
		void Scale(Vector3 newScale);

	private:
		Object();
		~Object() = default;

		std::vector<Model> m_Models;
		std::set<Behavior*> m_Behaviors;

		std::string m_Name;
		Vector3 m_Position;
		Quaternion m_Rotation;
		Vector3 m_Scale;
	};

#pragma warning(pop)
}