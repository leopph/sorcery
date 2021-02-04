#pragma once

#include "leopphapi.h"
#include "model.h"
#include "behavior.h"
#include "vector.h"

#include <set>
#include <vector>
#include <string>
#include <memory>

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

		const std::vector<Model>& Models() const;
		void AddModel(Model&& model);
		void RemoveModel(size_t index);

		const std::set<Behavior*>& Behaviors() const;
		template<class T> Behavior* AddBehavior();
		void RemoveBehavior(Behavior* behavior);
		template<class T> Behavior* GetBehavior() const;

		const std::string& Name() const;
		void Name(std::string newName);

		const Vector3& Position() const;
		void Position(Vector3 newPos);

		const Vector3& Rotation() const;
		void Rotation(Vector3 newRot);

		const Vector3& Scale() const;
		void Scale(Vector3 newScale);

	private:
		struct Deleter
		{
			void operator()(Object* object) const;
		};

		struct Comparator
		{
			using is_transparent = void;

			bool operator()(const std::unique_ptr<Object, Deleter>& left, const std::unique_ptr<Object, Deleter>& right) const;
			bool operator()(const std::unique_ptr<Object, Deleter>& left, const std::string& right) const;
			bool operator()(const std::string& left, const std::unique_ptr<Object, Deleter>& right) const;
			bool operator()(const std::unique_ptr<Object, Deleter>& left, const Object* right) const;
			bool operator()(const Object* left, const std::unique_ptr<Object, Deleter>& right) const;
		};

		Object();
		~Object() = default;

		static std::set<std::unique_ptr<Object, Deleter>, Comparator> s_Instances;

		std::vector<Model> m_Models;
		std::set<Behavior*> m_Behaviors;

		std::string m_Name;
		Vector3 m_Position;
		Vector3 m_Rotation;
		Vector3 m_Scale;

	public:
		static const std::set<std::unique_ptr<Object, Deleter>, Comparator>& Instances();

	};

#pragma warning(pop)
}