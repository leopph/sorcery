#pragma once

#include "leopphapi.h"
#include "model.h"
#include "behavior.h"

#include <set>
#include <vector>
#include <glm/glm.hpp>
#include <string>

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS REPRESENTING A GAME OBJECT
	class LEOPPHAPI Object final
	{
	private:

		// Comparator for fast name-based lookup
		struct Compare
		{
			using is_transparent = void;

			bool operator()(const Object* left, const Object* right) const;
			bool operator()(const Object* left, const std::string& right) const;
			bool operator()(const std::string& left, const Object* right) const;
		};


	public:
		static const std::set<Object*, Compare>& Instances();
		static Object* Get(const std::string& name);


		// MODEL RELATED
		const std::vector<Model>& Models() const;
		void AddModel(Model&& model);
		void RemoveModel(size_t index);


		// BEHAVIOR RELATED
		const std::set<Behavior*>& Behaviors() const;

		template<class T>
		Behavior* AddBehavior();

		void RemoveBehavior(Behavior* behavior);

		template<class T>
		Behavior* GetBehavior() const;


		// STATE RELATED
		const std::string& Name() const;
		void Name(std::string newName);

		const glm::vec3& Position() const;
		void Position(glm::vec3 newPos);


		const glm::vec3& Rotation() const;
		void Rotation(glm::vec3 newRot);
		

		const glm::vec3& Scale() const;
		void Scale(glm::vec3 newScale);


	private:
		static std::set<Object*, Compare> s_Instances;

		Object();
		~Object();


		// MODELS TO RENDER
		std::vector<Model> m_Models;


		// BEHAVIORS TO UPDATE
		std::set<Behavior*> m_Behaviors;


		// STATES OF THE OBJECT
		std::string m_Name;
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_Rotation{ 0.0f };
		glm::vec3 m_Scale{ 1.0f };
	};

#pragma warning(pop)
}