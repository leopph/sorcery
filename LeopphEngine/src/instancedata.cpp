#include "instancedata.h"

namespace leopph::implementation
{
	// comparator class ordering
	bool InstanceData::ObjectComparator::operator()(const Object* left, const Object* right) const
	{
		return left->Name() < right->Name();
	}

	bool InstanceData::ObjectComparator::operator()(const Object* left, const std::string& right) const
	{
		return left->Name() < right;
	}

	bool InstanceData::ObjectComparator::operator()(const std::string& left, const Object* right) const
	{
		return left < right->Name();
	}





	// static init
	std::unordered_map<unsigned, size_t> InstanceData::s_Textures{};
	std::unordered_map<unsigned, size_t> InstanceData::s_Meshes{};
	std::set<Object*, InstanceData::ObjectComparator> InstanceData::s_Objects{};






	void InstanceData::DestroyAll()
	{
		for (Object* object : s_Objects)
			delete object;

		s_Meshes.clear();
		s_Textures.clear();
	}





	void InstanceData::AddObject(Object* object)
	{
		s_Objects.emplace(object);
	}

	void InstanceData::RemoveObject(Object* object)
	{
		s_Objects.erase(s_Objects.find(object));
	}

	Object* InstanceData::FindObject(const std::string& name)
	{
		auto it = s_Objects.find(name);
		return it != s_Objects.end() ? *it : nullptr;
	}

	const std::set<Object*, InstanceData::ObjectComparator>& InstanceData::Objects()
	{
		return s_Objects;
	}

	void InstanceData::UpdateObjectKey(std::string oldKey, std::string&& newKey, std::function<void(Object*, std::string&&)> updater)
	{
		auto node = s_Objects.extract(s_Objects.find(oldKey));
		updater(node.value(), std::move(newKey));

		auto result = s_Objects.insert(std::move(node));

		if (!result.inserted)
		{
			updater(result.node.value(), std::move(oldKey));
			s_Objects.insert(std::move(result.node));
		}
	}






	void InstanceData::AddTexture(unsigned id)
	{
		s_Textures.try_emplace(id, 0);
		s_Textures[id]++;
	}

	void InstanceData::RemoveTexture(unsigned id)
	{
		s_Textures[id]--;
	}

	std::size_t InstanceData::TextureCount(unsigned id)
	{
		return s_Textures[id];
	}



	
	void InstanceData::AddMesh(unsigned id)
	{
		s_Meshes.try_emplace(id, 0);
		s_Meshes[id]++;
	}

	void InstanceData::RemoveMesh(unsigned id)
	{
		s_Meshes[id]--;
	}

	std::size_t InstanceData::MeshCount(unsigned id)
	{
		return s_Meshes[id];
	}
}