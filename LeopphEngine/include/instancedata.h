#pragma once

#include <memory>
#include <functional>
#include <set>
#include <string>
#include <cstddef>

#include "object.h"

namespace leopph::implementation
{
	class InstanceData
	{
	private:
		class ObjectComparator
		{
		public:
			using is_transparent = void;

			bool operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const;
			bool operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const std::string& right) const;
			bool operator()(const std::string& left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const;
			bool operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const Object* right) const;
			bool operator()(const Object* left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const;
		};


	public:
		static void AddObject(Object* object, std::function<void(Object*)>&& deleter);
		static void RemoveObject(Object* object);
		static Object* FindObject(const std::string& name);
		static const std::set<std::unique_ptr<Object, std::function<void(Object*)>>, ObjectComparator>& Objects();
		static void UpdateObjectKey(std::string&& oldKey, std::string&& newKey, std::function<void(Object*, std::string&&)> updater);
		
		static void AddTexture(unsigned id);
		static void RemoveTexture(unsigned id);
		static std::size_t TextureCount(unsigned id);

		static void AddMesh(unsigned id);
		static void RemoveMesh(unsigned id);
		static std::size_t MeshCount(unsigned id);


	private:
		static std::unordered_map<unsigned, std::size_t> s_Textures;
		static std::unordered_map<unsigned, size_t> s_Meshes;
		static std::set<std::unique_ptr<Object, std::function<void(Object*)>>, ObjectComparator> s_Objects;
	};
}