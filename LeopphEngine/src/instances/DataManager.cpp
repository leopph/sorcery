#include "DataManager.hpp"

#include "../util/logger.h"

#include <stdexcept>
#include <tuple>

namespace leopph::impl
{
	std::unique_ptr<AmbientLight> DataManager::s_AmbientLight{ nullptr };
	DirectionalLight* DataManager::s_DirLight{ nullptr };
	std::vector<PointLight*> DataManager::s_PointLights{};
	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};

	std::map<Object*, std::set<Component*>, ObjectLess> DataManager::s_Objects{};
	std::set<Behavior*> DataManager::s_Behaviors{};

	std::unordered_set<TextureReference, TextureHash, TextureEqual> DataManager::s_Textures{};
	std::unordered_map<std::filesystem::path, ModelReference, PathHash> DataManager::s_Models{};

	std::unordered_map<SkyboxImpl, std::size_t, SkyboxImplHash, SkyboxImplEqual> DataManager::s_Skyboxes{};

	std::list<ShadowMap> DataManager::s_ShadowMaps{};

	std::unordered_map<const Object*, std::pair<const Matrix4, const Matrix4>> DataManager::s_MatrixCache{};

	std::unordered_map<unsigned, std::size_t> DataManager::s_Buffers{};

	
	void DataManager::DestroyAllObjects()
	{
		for (auto it = s_Objects.begin(); it != s_Objects.end();)
		{
			delete it->first;
			it = s_Objects.begin();
		}

		s_ShadowMaps.clear();
	}


	
	void DataManager::RegisterObject(Object* object)
	{
		s_Objects.try_emplace(object);
	}

	void DataManager::UnregisterObject(Object* object)
	{
		s_Objects.erase(object);
		s_MatrixCache.erase(object);
	}

	Object* DataManager::FindObject(const std::string& name)
	{
		const auto it = s_Objects.find(name);
		return it != s_Objects.end() ? it->first : nullptr;
	}

	const std::map<Object*, std::set<Component*>, ObjectLess>& DataManager::Objects()
	{
		return s_Objects;
	}

	
	bool DataManager::IsTextureStored(const std::filesystem::path& path)
	{
		return s_Textures.contains(path);
	}

	std::unique_ptr<Texture> DataManager::CreateTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return std::make_unique<Texture>(*s_Textures.find(path));
	}

	void DataManager::StoreTextureRef(const Texture& other)
	{
		if (s_Textures.contains(other.path))
		{
			const auto msg{ "Texture on path [" + other.path.string() + "] has already been loaded." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.emplace(other.path, other.id, other.isTransparent, 1);
	}

	void DataManager::IncTexture(const std::filesystem::path& path)
	{
		if (!s_Textures.contains(path))
		{
			const auto msg{ "Texture on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Textures.find(path)->count++;
	}

	void DataManager::DecTexture(const std::filesystem::path& path)
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



	const std::set<Behavior*>& DataManager::Behaviors()
	{
		return s_Behaviors;
	}

	void DataManager::RegisterBehavior(Behavior* behavior)
	{
		s_Behaviors.insert(behavior);
	}

	void DataManager::UnregisterBehavior(Behavior* behavior)
	{
		s_Behaviors.erase(behavior);
	}

	const std::set<Component*>& DataManager::Components(Object* object)
	{
		return s_Objects[object];
	}

	void DataManager::RegisterComponent(Component* component)
	{
		s_Objects[&component->object].insert(component);
	}

	void DataManager::UnregisterComponent(Component* component)
	{
		s_Objects[&component->object].erase(component);
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

	void DataManager::RegisterPointLight(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}

	void DataManager::UnregisterPointLight(PointLight* pointLight)
	{
		for (auto it = s_PointLights.begin(); it != s_PointLights.end(); ++it)
			if (*it == pointLight)
			{
				s_PointLights.erase(it);
				return;
			}
	}

	const AssimpModelImpl& DataManager::GetModelReference(const std::filesystem::path& path)
	{
		if (!s_Models.contains(path))
			return (*s_Models.emplace(path, path).first).second.ReferenceModel();

		return (*s_Models.find(path)).second.ReferenceModel();
	}

	void DataManager::IncModel(const std::filesystem::path& path, Object* object)
	{
		if (!s_Models.contains(path))
		{
			const auto errorMsg{ "Model on path [" + path.string() + "] has not been loaded yet." };
			Logger::Instance().Error(errorMsg);
			throw std::runtime_error(errorMsg);
		}

		s_Models.at(path).AddObject(object);
	}

	void DataManager::DecModel(const std::filesystem::path& path, Object* object)
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

	const std::unordered_map<std::filesystem::path, ModelReference, PathHash>& DataManager::Models()
	{
		return s_Models;
	}

	const leopph::impl::SkyboxImpl* DataManager::GetSkybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front)
	{
		const auto fileNames{ left.string() + right.string() + top.string() + bottom.string() + back.string() + front.string() };
		auto it{ s_Skyboxes.find(fileNames) };

		if (it == s_Skyboxes.end())
			return nullptr;

		return &it->first;
	}

	const leopph::impl::SkyboxImpl& DataManager::GetSkybox(const Skybox& skybox)
	{
		if (!s_Skyboxes.contains(skybox))
		{
			const auto msg{ "The requested skybox does not exist." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		return s_Skyboxes.find(skybox)->first;
	}

	const SkyboxImpl* DataManager::RegisterSkybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front)
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

	void DataManager::IncSkybox(const SkyboxImpl* skybox)
	{
		if (!s_Skyboxes.contains(*skybox))
		{
			const auto msg{ "Skybox with ID [" + std::to_string(skybox->ID()) + "] is not yet registered." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		s_Skyboxes.at(*skybox)++;
	}

	void DataManager::DecSkybox(const SkyboxImpl* skybox)
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

	leopph::AmbientLight* DataManager::AmbientLight()
	{
		return s_AmbientLight.get();
	}

	void DataManager::AmbientLight(leopph::AmbientLight*&& light)
	{
		s_AmbientLight = std::unique_ptr<leopph::AmbientLight>(std::forward<leopph::AmbientLight*>(light));
	}


	const std::pair<const Matrix4, const Matrix4>& DataManager::ModelAndNormalMatrices(const Object* const object)
	{
		if (!object->isStatic)
		{
			const auto msg{ "Trying to access cached model matrix for dynamic object [" + object->name + "]." };
			Logger::Instance().Warning(msg);
			throw std::runtime_error{ msg };
		}

		if (auto it = s_MatrixCache.find(object); it != s_MatrixCache.end())
			return it->second;

		Matrix4 modelMatrix = Matrix4::Scale(object->Transform().Scale());
		modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
		modelMatrix *= Matrix4::Translate(object->Transform().Position());

		return s_MatrixCache.emplace(object, std::make_pair(modelMatrix, modelMatrix.Inverse().Transposed())).first->second;
	}

	const std::list<ShadowMap>& DataManager::ShadowMaps()
	{
		return s_ShadowMaps;
	}

	void DataManager::CreateShadowMap(const Vector2& resolution)
	{
		s_ShadowMaps.emplace_back(resolution);
	}

	void DataManager::DeleteShadowMap()
	{
		if (!s_ShadowMaps.empty())
		{
			s_ShadowMaps.pop_back();
		}
	}

	const std::unordered_set<const SpotLight*>& DataManager::SpotLights()
	{
		return s_SpotLights;
	}

	void DataManager::RegisterSpotLight(const SpotLight* spotLight)
	{
		s_SpotLights.emplace(spotLight);
	}


	void DataManager::UnregisterSpotLight(const SpotLight* spotLight)
	{
		s_SpotLights.erase(spotLight);
	}


	void DataManager::RegisterBuffer(const RefCountedBuffer& buffer)
	{
		s_Buffers.try_emplace(buffer.name, 0);
		s_Buffers[buffer.name]++;
	}


	void DataManager::UnregisterBuffer(const RefCountedBuffer& buffer)
	{
		if (s_Buffers.contains(buffer.name))
		{
			s_Buffers[buffer.name]--;
		}
		else
		{
			Logger::Instance().Warning("Trying to unregister buffer [" + std::to_string(buffer.name) + "] but it is not registered.");
		}
	}


	std::size_t DataManager::ReferenceCount(const RefCountedBuffer& buffer)
	{
		const auto it{ s_Buffers.find(buffer.name) };

		if (it == s_Buffers.end())
		{
			return 0;
		}

		return it->second;
	}

}