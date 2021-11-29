#include "DataManager.hpp"

#include "../util/logger.h"


namespace leopph::impl
{
	std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual> DataManager::s_EntitiesAndComponents{};
	std::unordered_set<Behavior*> DataManager::s_Behaviors{};
	DirectionalLight* DataManager::s_DirLight{nullptr};
	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};
	std::vector<PointLight*> DataManager::s_PointLights{};
	std::unordered_map<const Transform*, std::pair<Matrix4, Matrix4>> DataManager::s_Matrices{};
	std::unordered_map<ModelImpl, std::unordered_set<Model*>, PathedHash<ModelImpl>, PathedEqual<ModelImpl>> DataManager::s_Models{};
	std::unordered_map<TextureImpl, std::unordered_set<Texture*>, PathedHash<TextureImpl>, PathedEqual<TextureImpl>> DataManager::s_Textures{};
	std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> DataManager::s_Skyboxes{};


	void DataManager::Clear()
	{
		for (auto it = s_EntitiesAndComponents.begin(); it != s_EntitiesAndComponents.end();)
		{
			delete it->first;
			it = s_EntitiesAndComponents.begin();
		}

		Logger::Instance().Debug("All data have been cleared.");
	}


	void DataManager::Register(Entity* entity)
	{
		s_EntitiesAndComponents.try_emplace(entity);
	}


	void DataManager::Unregister(Entity* entity)
	{
		s_EntitiesAndComponents.erase(entity);
	}


	Entity* DataManager::Find(const std::string& name)
	{
		const auto it = s_EntitiesAndComponents.find(name);
		return it != s_EntitiesAndComponents.end() ? it->first : nullptr;
	}


	const std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual>& DataManager::EntitiesAndComponents()
	{
		return s_EntitiesAndComponents;
	}


	const std::unordered_set<Behavior*>& DataManager::Behaviors()
	{
		return s_Behaviors;
	}


	void DataManager::Register(Behavior* behavior)
	{
		s_Behaviors.insert(behavior);
	}


	void DataManager::Unregister(Behavior* behavior)
	{
		s_Behaviors.erase(behavior);
	}


	const std::unordered_set<Component*>& DataManager::Components(Entity* entity)
	{
		return s_EntitiesAndComponents[entity];
	}


	void DataManager::Register(Component* component)
	{
		s_EntitiesAndComponents[&component->Entity].insert(component);
	}


	void DataManager::Unregister(Component* component)
	{
		s_EntitiesAndComponents[&component->Entity].erase(component);
	}


	DirectionalLight* DataManager::DirectionalLight()
	{
		return s_DirLight;
	}


	void DataManager::DirectionalLight(leopph::DirectionalLight* dirLight)
	{
		s_DirLight = dirLight;
	}


	const std::vector<PointLight*>& DataManager::PointLights()
	{
		return s_PointLights;
	}


	void DataManager::Register(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}


	void DataManager::Unregister(PointLight* pointLight)
	{
		for (auto it = s_PointLights.begin(); it != s_PointLights.end(); ++it)
		{
			if (*it == pointLight)
			{
				s_PointLights.erase(it);
				return;
			}
		}
	}


	const std::unordered_set<const SpotLight*>& DataManager::SpotLights()
	{
		return s_SpotLights;
	}


	void DataManager::Register(const SpotLight* spotLight)
	{
		s_SpotLights.emplace(spotLight);
	}


	void DataManager::Unregister(const SpotLight* spotLight)
	{
		s_SpotLights.erase(spotLight);
	}


	void DataManager::StoreMatrices(const Transform* transform, const Matrix4& model, const Matrix4& normal)
	{
		s_Matrices.insert_or_assign(transform, std::pair<Matrix4, Matrix4>{model, normal});
	}


	void DataManager::DiscardMatrices(const Transform* transform)
	{
		s_Matrices.erase(transform);
	}


	const std::pair<Matrix4, Matrix4>& DataManager::GetMatrices(const Transform* transform)
	{
		return s_Matrices.at(transform);
	}

	ModelImpl* DataManager::CreateOrGetModelImpl(std::filesystem::path path)
	{
		return &const_cast<ModelImpl&>(s_Models.emplace(std::move(path), std::unordered_set<Model*>{}).first->first);
	}

	void DataManager::DestroyModelImpl(ModelImpl* const model)
	{
		s_Models.erase(*model);
	}

	void DataManager::RegisterModelComponent(ModelImpl* const model, Model* const component)
	{
		s_Models.at(*model).insert(component);
	}

	void DataManager::UnregisterModelComponent(ModelImpl* const model, Model* const component)
	{
		s_Models.at(*model).erase(component);
	}

	const std::unordered_map<ModelImpl, std::unordered_set<Model*>, PathedHash<ModelImpl>, PathedEqual<ModelImpl>>& DataManager::Models()
	{
		return s_Models;
	}

	TextureImpl* DataManager::CreateOrGetTextureImpl(std::filesystem::path path)
	{
		return &const_cast<TextureImpl&>(s_Textures.emplace(std::move(path), std::unordered_set<Texture*>{}).first->first);
	}

	void DataManager::DestroyTextureImpl(TextureImpl* const texture)
	{
		s_Textures.erase(*texture);
	}

	void DataManager::RegisterTextureHandle(TextureImpl* const texture, Texture* const handle)
	{
		s_Textures.at(*texture).insert(handle);
	}

	void DataManager::UnregisterTextureHandle(TextureImpl* const texture, Texture* const handle)
	{
		s_Textures.at(*texture).erase(handle);
	}

	const std::unordered_map<TextureImpl, std::unordered_set<Texture*>, PathedHash<TextureImpl>, PathedEqual<TextureImpl>>& DataManager::Textures()
	{
		return s_Textures;
	}

	SkyboxImpl* DataManager::CreateOrGetSkyboxImpl(std::filesystem::path allPaths)
	{
		return &const_cast<SkyboxImpl&>(s_Skyboxes.emplace(std::move(allPaths), std::unordered_set<Skybox*>{}).first->first);
	}

	void DataManager::DestroySkyboxImpl(SkyboxImpl* const skybox)
	{
		s_Skyboxes.erase(*skybox);
	}

	void DataManager::RegisterSkyboxHandle(SkyboxImpl* const skybox, Skybox* const handle)
	{
		s_Skyboxes.at(*skybox).insert(handle);
	}

	void DataManager::UnregisterSkyboxHandle(SkyboxImpl* const skybox, Skybox* const handle)
	{
		s_Skyboxes.at(*skybox).erase(handle);
	}

	const std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>>& DataManager::Skyboxes()
	{
		return s_Skyboxes;
	}
}
