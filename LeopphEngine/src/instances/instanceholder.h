#pragma once

#include <set>
#include <string>
#include <cstddef>
#include <functional>
#include "../hierarchy/object.h"
#include "../components/lighting/dirlight.h"
#include "../components/lighting/pointlight.h"
#include <vector>

namespace leopph::impl
{
	class InstanceHolder
	{
		class ObjectComparator
		{
		public:
			using is_transparent = void;

			bool operator()(const Object* left, const Object* right) const;
			bool operator()(const Object* left, const std::string& right) const;
			bool operator()(const std::string& left, const Object* right) const;
		};


	public:
		static void DestroyAll();

		static const std::set<Object*, ObjectComparator>& Objects();
		static void AddObject(Object* object);
		static void RemoveObject(Object* object);
		static Object* FindObject(const std::string& name);
		
		static std::size_t TextureCount(unsigned id);
		static void AddTexture(unsigned id);
		static void RemoveTexture(unsigned id);

		static std::size_t MeshCount(unsigned id);
		static void AddMesh(unsigned id);
		static void RemoveMesh(unsigned id);

		static const std::set<Behavior*>& Behaviors();
		static void AddBehavior(Behavior* behavior);
		static void RemoveBehavior(Behavior* behavior);

		static const std::set<Component*>& Components();
		static void AddComponent(Component* component);
		static void RemoveComponent(Component* component);

		static DirectionalLight* DirectionalLight();
		static void DirectionalLight(leopph::DirectionalLight* dirLight);

		static const std::vector<PointLight*>& PointLights();
		static void AddPointLight(PointLight* pointLight);
		static void RemovePointLight(PointLight* pointLight);

	private:
		static std::unordered_map<unsigned, std::size_t> s_Textures;
		static std::unordered_map<unsigned, size_t> s_Meshes;
		static std::set<Object*, ObjectComparator> s_Objects;
		static std::set<Behavior*> s_Behaviors;
		static std::set<Component*> s_Components;
		static leopph::DirectionalLight* s_DirLight;
		static std::vector<PointLight*> s_PointLights;
	};
}