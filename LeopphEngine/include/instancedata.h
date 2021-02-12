#pragma once

#include <memory>
#include <functional>
#include <set>
#include <string>

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


	private:
		static std::set<std::unique_ptr<Object, std::function<void(Object*)>>, ObjectComparator> s_Objects;
	};
}