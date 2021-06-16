#include "instanceholder.h"

namespace leopph::impl
{
	bool InstanceHolder::ObjectComparator::operator()(const Object* left, const Object* right) const
	{
		return left->Name() < right->Name();
	}

	bool InstanceHolder::ObjectComparator::operator()(const Object* left, const std::string& right) const
	{
		return left->Name() < right;
	}

	bool InstanceHolder::ObjectComparator::operator()(const std::string& left, const Object* right) const
	{
		return left < right->Name();
	}

	
	std::unordered_map<unsigned, size_t> InstanceHolder::s_Textures{};
	std::unordered_map<unsigned, size_t> InstanceHolder::s_Meshes{};
	std::map<Object*, std::set<Component*>, InstanceHolder::ObjectComparator> InstanceHolder::s_Objects{};
	std::set<Behavior*> InstanceHolder::s_Behaviors{};
	DirectionalLight* InstanceHolder::s_DirLight{ nullptr };
	std::vector<PointLight*> InstanceHolder::s_PointLights{};

	
	void InstanceHolder::DestroyAll()
	{
		for (const auto& pair : s_Objects)
		{
			for (const auto component : pair.second)
				delete component;
			delete pair.first;
		}

		s_Meshes.clear();
		s_Textures.clear();
	}


	
	void InstanceHolder::AddObject(Object* object)
	{
		s_Objects.try_emplace(object);
	}

	void InstanceHolder::RemoveObject(Object* object)
	{
		const auto it = s_Objects.find(object);

		for (const auto component : it->second)
			delete component;
		delete object;
		
		s_Objects.erase(it);
	}

	Object* InstanceHolder::FindObject(const std::string& name)
	{
		const auto it = s_Objects.find(name);
		return it != s_Objects.end() ? it->first : nullptr;
	}

	const std::map<Object*, std::set<Component*>, InstanceHolder::ObjectComparator>& InstanceHolder::Objects()
	{
		return s_Objects;
	}

	

	void InstanceHolder::AddTexture(unsigned id)
	{
		s_Textures.try_emplace(id, 0);
		s_Textures[id]++;
	}

	void InstanceHolder::RemoveTexture(unsigned id)
	{
		s_Textures[id]--;
	}

	std::size_t InstanceHolder::TextureCount(unsigned id)
	{
		return s_Textures[id];
	}


	
	void InstanceHolder::AddMesh(unsigned id)
	{
		s_Meshes.try_emplace(id, 0);
		s_Meshes[id]++;
	}

	void InstanceHolder::RemoveMesh(unsigned id)
	{
		s_Meshes[id]--;
	}

	std::size_t InstanceHolder::MeshCount(unsigned id)
	{
		return s_Meshes[id];
	}



	const std::set<Behavior*>& InstanceHolder::Behaviors()
	{
		return s_Behaviors;
	}

	void InstanceHolder::AddBehavior(Behavior* behavior)
	{
		s_Behaviors.insert(behavior);
	}

	void InstanceHolder::RemoveBehavior(Behavior* behavior)
	{
		s_Behaviors.erase(behavior);
	}

	const std::set<Component*>& InstanceHolder::Components(Object* object)
	{
		return s_Objects[object];
	}

	void InstanceHolder::AddComponent(Component* component)
	{
		s_Objects[&component->Object()].insert(component);
	}

	void InstanceHolder::RemoveComponent(Component* component)
	{
		s_Objects[&component->Object()].erase(component);
		delete component;
	}

	DirectionalLight* InstanceHolder::DirectionalLight()
	{
		return s_DirLight;
	}

	void InstanceHolder::DirectionalLight(leopph::DirectionalLight* dirLight)
	{
		s_DirLight = dirLight;
	}

	const std::vector<PointLight*>& InstanceHolder::PointLights()
	{
		return s_PointLights;
	}

	void InstanceHolder::AddPointLight(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}

	void InstanceHolder::RemovePointLight(PointLight* pointLight)
	{
		for (auto it = s_PointLights.begin(); it != s_PointLights.end(); ++it)
			if (*it == pointLight)
			{
				s_PointLights.erase(it);
				return;
			}
	}
}