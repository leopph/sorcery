#pragma once

#include <set>
#include <string>
#include <cstddef>
#include <map>
#include <functional>

#include "../hierarchy/object.h"

namespace leopph::impl
{
	class InstanceData
	{
	private:
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
		static void UpdateObjectKey(std::string oldKey, std::string&& newKey, std::function<void(Object*, std::string&&)> updater);
		
		static std::size_t TextureCount(unsigned id);
		static void AddTexture(unsigned id);
		static void RemoveTexture(unsigned id);

		static std::size_t MeshCount(unsigned id);
		static void AddMesh(unsigned id);
		static void RemoveMesh(unsigned id);


	private:
		static std::unordered_map<unsigned, std::size_t> s_Textures;
		static std::unordered_map<unsigned, size_t> s_Meshes;
		static std::set<Object*, ObjectComparator> s_Objects;
	};
}