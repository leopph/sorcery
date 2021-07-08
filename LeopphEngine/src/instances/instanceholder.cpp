#include "instanceholder.h"

#include "../util/logger.h"

#include <stdexcept>
#include <tuple>
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
	std::unordered_map<SkyboxImpl, std::size_t, SkyboxImplHash, SkyboxImplEqual> InstanceHolder::s_Skyboxes{};

	
	void InstanceHolder::DestroyAllObjects()
	{
		for (const auto& pair : s_Objects)
		{
			for (const auto component : pair.second)
				delete component;
			delete pair.first;
		}
	}


	
	void InstanceHolder::StoreObject(Object* object)
	{
		s_Objects.try_emplace(object);
	}

	void InstanceHolder::DeleteObject(Object* object)
	{
		const auto it = s_Objects.find(object);

		for (const auto component : it->second)
			delete component;
		delete object;
		
		s_Objects.erase(it);
	}

	void InstanceHolder::RenameObject(Object* object, std::string name)
	{
		if (FindObject(name) != nullptr)
		{
			auto msg{ "Cannot rename Object [" + object->Name() + "] to [" + name + "] because the new name is already in use." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		auto node = s_Objects.extract(object);
		node.key()->m_Name = std::move(name);
		s_Objects.insert(std::move(node));
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

	std::unique_ptr<Texture> InstanceHolder::CreateTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return std::make_unique<Texture>(*s_Textures.find(path));
	}

	void InstanceHolder::StoreTextureRef(const Texture& other)
	{
		if (s_Textures.contains(other.path))
		{
			const auto msg{ "Texture on path [" + other.path.string() + "] has already been loaded." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.emplace(other.path, other.id, other.isTransparent, 1);
	}

	void InstanceHolder::IncTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.find(path)->count++;
	}

	void InstanceHolder::DecTexture(const std::filesystem::path& path)
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

	void InstanceHolder::RegisterComponent(Component* component)
	{
		s_Objects[&component->Object()].insert(component);
	}

	void InstanceHolder::UnregisterComponent(Component* component)
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

	void InstanceHolder::RegisterPointLight(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}

	void InstanceHolder::UnregisterPointLight(PointLight* pointLight)
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

	void InstanceHolder::IncModel(const std::filesystem::path& path, Object* object)
	{
		if (!s_Models.contains(path))
		{
			const auto errorMsg{ "Model on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(errorMsg);
			throw std::runtime_error(errorMsg);
		}

		s_Models.at(path).AddObject(object);
	}

	void InstanceHolder::DecModel(const std::filesystem::path& path, Object* object)
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

	void InstanceHolder::IncMesh(unsigned id)
	{
		if (id == 0)
			return;

		if (!s_MeshCounts.contains(id))
			s_MeshCounts.insert({ id, 1 });
		else
			s_MeshCounts.at(id)++;
	}

	void InstanceHolder::DecMesh(unsigned id)
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

	const leopph::impl::SkyboxImpl* InstanceHolder::GetSkybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front)
	{
		const auto fileNames{ left.string() + right.string() + top.string() + bottom.string() + back.string() + front.string() };
		auto it{ s_Skyboxes.find(fileNames) };

		if (it == s_Skyboxes.end())
			return nullptr;

		return &it->first;
	}

	const leopph::impl::SkyboxImpl& InstanceHolder::GetSkybox(const Skybox& skybox)
	{
		if (!s_Skyboxes.contains(skybox))
		{
			const auto msg{ "The requested skybox does not exist." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return s_Skyboxes.find(skybox)->first;
	}

	const SkyboxImpl* InstanceHolder::RegisterSkybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front)
	{
		const auto fileNames{ left.string() + ";" + right.string() + ";" + top.string() + ";" + bottom.string() + ";" + back.string() + ";" + front.string()};

		if (s_Skyboxes.contains(fileNames))
		{
			const auto msg{"Skybox of files [" + fileNames + "] is already registered."};

			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return &s_Skyboxes.emplace(std::piecewise_construct, std::make_tuple(left, right, top, bottom, back, front), std::make_tuple(1)).first->first;
	}

	void InstanceHolder::IncSkybox(const SkyboxImpl* skybox)
	{
		if (!s_Skyboxes.contains(*skybox))
		{
			const auto msg{ "Skybox with ID [" + std::to_string(skybox->ID()) + "] is not yet registered." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Skyboxes.at(*skybox)++;
	}

	void InstanceHolder::DecSkybox(const SkyboxImpl* skybox)
	{
		if (!s_Skyboxes.contains(*skybox))
		{
			const auto msg{ "Skybox with ID [" + std::to_string(skybox->ID()) + "] is not yet registered." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Skyboxes.at(*skybox)--;

		if (s_Skyboxes.at(*skybox) == 0)
			s_Skyboxes.erase(*skybox);
	}
}