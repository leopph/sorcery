#pragma once

#include "leopph.h"
#include "model.h"

#include <set>
#include <vector>
#include <glm/glm.hpp>

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS REPRESENTING A GAME OBJECT
	class LEOPPHAPI Object
	{
	public:
		// RETURNS A CONTAINER OF ALL INSTANTIATED OBJECTS
		static const std::set<Object*>& Instances();

		Object();
		virtual ~Object();

		// MODEL RELATED
		const std::vector<Model>& Models() const;
		void AddModel(Model&& model);
		void RemoveModel(size_t index);

		// STATE RELATED
		const glm::vec3& Position() const;
		void Position(glm::vec3 newPos);

		const glm::vec3& Rotation() const;
		void Rotation(glm::vec3 newRot);
		
		const glm::vec3& Scale() const;
		void Scale(glm::vec3 newScale);

		// THIS IS CALLED EVERY FRAME ON ALL OBJECTS
		virtual void Update();

	protected:
		// MODELS TO RENDER
		std::vector<Model> m_Models;

		// STATES OF THE OBJECT
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_Rotation{ 0.0f };
		glm::vec3 m_Scale{ 1.0f };

	private:
		static std::set<Object*> s_Instances;
	};

#pragma warning(pop)
}