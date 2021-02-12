#include "instancedata.h"

namespace leopph::implementation
{
	// comparator class ordering
	bool InstanceData::ObjectComparator::operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const
	{
		return left->Name() < right->Name();
	}

	bool InstanceData::ObjectComparator::operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const std::string& right) const
	{
		return left->Name() < right;
	}

	bool InstanceData::ObjectComparator::operator()(const std::string& left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const
	{
		return left < right->Name();
	}

	bool InstanceData::ObjectComparator::operator()(const std::unique_ptr<Object, std::function<void(Object*)>>& left, const Object* right) const
	{
		return left->Name() < right->Name();
	}

	bool InstanceData::ObjectComparator::operator()(const Object* left, const std::unique_ptr<Object, std::function<void(Object*)>>& right) const
	{
		return left->Name() < right->Name();
	}





	// static init
	std::set<std::unique_ptr<Object, std::function<void(Object*)>>, InstanceData::ObjectComparator> InstanceData::s_Objects{};




	void InstanceData::AddObject(Object* object, std::function<void(Object*)>&& deleter)
	{
		s_Objects.emplace(object, std::move(deleter));
	}

	void InstanceData::RemoveObject(Object* object)
	{
		s_Objects.erase(s_Objects.find(object));
	}

	Object* InstanceData::FindObject(const std::string& name)
	{
		auto it = s_Objects.find(name);
		return it != s_Objects.end() ? it->get() : nullptr;
	}

	const std::set<std::unique_ptr<Object, std::function<void(Object*)>>, InstanceData::ObjectComparator>& InstanceData::Objects()
	{
		return s_Objects;
	}

	void InstanceData::UpdateObjectKey(std::string&& oldKey, std::string&& newKey, std::function<void(Object*, std::string&&)> updater)
	{
		auto node = s_Objects.extract(s_Objects.find(oldKey));
		updater(node.value().get(), std::move(newKey));

		auto result = s_Objects.insert(std::move(node));

		if (!result.inserted)
		{
			updater(result.node.value().get(), std::move(oldKey));
			s_Objects.insert(std::move(result.node));
		}
	}
}