#include "instanceholder.h"

#include "../util/logger.h"

#include <stdexcept>
#include <utility>

namespace leopph::impl
{
	std::unordered_set<TextureReference, TextureHash, TextureEqual> InstanceHolder::s_Textures{};
	std::map<Object*, std::set<Component*>, ObjectComparator> InstanceHolder::s_Objects{};
	std::set<Behavior*> InstanceHolder::s_Behaviors{};
	DirectionalLight* InstanceHolder::s_DirLight{ nullptr };
	std::vector<PointLight*> InstanceHolder::s_PointLights{};
	std::unordered_map<unsigned, std::size_t> InstanceHolder::s_MeshCounts{};
	std::unordered_map<std::filesystem::path, ModelReference> InstanceHolder::s_Models{};

	
	void InstanceHolder::DestroyAll()
	{
		for (const auto& pair : s_Objects)
		{
			for (const auto component : pair.second)
				delete component;
			delete pair.first;
		}

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

	const std::map<Object*, std::set<Component*>, ObjectComparator>& InstanceHolder::Objects()
	{
		return s_Objects;
	}

	
	bool InstanceHolder::IsTextureStored(const std::filesystem::path& path)
	{
		return s_Textures.contains(path);
	}

	std::unique_ptr<Texture> InstanceHolder::GetTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return std::make_unique<Texture>(*s_Textures.find(path));
	}

	void InstanceHolder::StoreTexture(const Texture& other)
	{
		if (s_Textures.contains(other.path))
		{
			const auto msg{ "Texture on path [" + other.path.string() + "] has already been loaded." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.emplace(TextureReference{ .path = other.path, .id = other.id, .count = 1 });
	}

	void InstanceHolder::AddTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.find(path)->count++;
	}

	void InstanceHolder::RemoveTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		const auto& it{ s_Textures.find(path) };

		it->count--;

		if (it->count == 0)
			s_Textures.erase(it);
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

	const AssimpModelImpl& InstanceHolder::GetModelReference(const std::filesystem::path& path)
	{
		if (!s_Models.contains(path))
			return (*s_Models.emplace(path, path).first).second.ReferenceModel();

		return (*s_Models.find(path)).second.ReferenceModel();
	}

	void InstanceHolder::RegisterModelObject(const std::filesystem::path& path, Object* object)
	{
		if (!s_Models.contains(path))
		{
			const auto errorMsg{ "Model on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(errorMsg);
			throw std::runtime_error(errorMsg);
		}

		s_Models.at(path).AddObject(object);
	}

	void InstanceHolder::UnregisterModelObject(const std::filesystem::path& path, Object* object)
	{
		if (!s_Models.contains(path))
		{
			const auto errorMsg{ "Model on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(errorMsg);
			throw std::runtime_error(errorMsg);
		}

		s_Models.at(path).RemoveObject(object);

		if (s_Models.at(path).ReferenceCount() == 0)
			s_Models.erase(path);
	}

	const std::unordered_map<std::filesystem::path, leopph::impl::ModelReference>& InstanceHolder::Models()
	{
		return s_Models;
	}

	std::size_t InstanceHolder::MeshCount(unsigned id)
	{
		if (s_MeshCounts.contains(id))
			return s_MeshCounts.at(id);
		
		return 0;
	}

	void InstanceHolder::AddMesh(unsigned id)
	{
		if (id == 0)
			return;

		if (!s_MeshCounts.contains(id))
			s_MeshCounts.insert({ id, 1 });
		else
			s_MeshCounts.at(id)++;
	}

	void InstanceHolder::RemoveMesh(unsigned id)
	{
		if (id == 0)
			return;

		if (!s_MeshCounts.contains(id))
		{
			const auto errorMsg{ "Mesh with ID [" + std::to_string(id) + "] has not been loaded yet." };
			Logger::Instance().Error(errorMsg);
			throw std::runtime_error(errorMsg);
		}

		s_MeshCounts.at(id)--;

		if (s_MeshCounts.at(id) == 0)
			s_MeshCounts.erase(id);
	}

}