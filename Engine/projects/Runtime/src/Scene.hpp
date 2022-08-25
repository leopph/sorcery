#pragma once

#include <span>
#include <string>
#include <vector>


namespace leopph
{
	class Scene
	{
		friend class Node;
		friend class SceneManager;

		public:
			[[nodiscard]] std::span<Node* const> get_entities() const;


		private:
			explicit Scene(std::string name);

		public:
			Scene(Scene const& other) = delete;
			Scene& operator=(Scene const& other) = delete;

			Scene(Scene&& other) = delete;
			Scene& operator=(Scene&& other) = delete;

			~Scene();


		private:
			void add(Node* entity);
			void remove(Node* entity);

			std::vector<Node*> mEntities;
			std::string mName;
	};
}
