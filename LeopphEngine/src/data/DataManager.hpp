#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../entity/Entity.hpp"
#include "../rendering/ShadowMap.hpp"
#include "../rendering/geometry/ModelResource.hpp"
#include "../util/equal/EntityEqual.hpp"
#include "../util/equal/UniqueResourceEqual.hpp"
#include "../util/hash/EntityHash.hpp"
#include "../util/hash/UniqueResourceHash.hpp"
#include "managed/Resource.hpp"
#include "managed/ResourceHandleBase.hpp"
#include "managed/UniqueResource.hpp"

#include <cstddef>
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>



namespace leopph::impl
{
	class DataManager
	{
		public:
			static void Clear();


			static void Register(Entity* entity);
			static void Register(Behavior* behavior);
			static void Register(Component* component);
			static void Register(PointLight* pointLight);
			static void Register(const ModelResource* model);
			static void Register(const Resource* resource);
			static void Register(const UniqueResource* resource);
			static void Register(const Resource* resource, const ResourceHandleBase* handle);
			static void Register(const UniqueResource* resource, const ResourceHandleBase* handle);
			static void Register(const SpotLight* spotLight);


			static void Unregister(Entity* entity);
			static void Unregister(Behavior* behavior);
			static void Unregister(Component* component);
			static void Unregister(PointLight* pointLight);
			static void Unregister(const ModelResource* model);
			static void Unregister(const Resource* resource);
			static void Unregister(const UniqueResource* resource);
			static void Unregister(const Resource* resource, const ResourceHandleBase* handle);
			static void Unregister(const UniqueResource* resource, const ResourceHandleBase* handle);
			static void Unregister(const SpotLight* spotLight);


			static Entity* Find(const std::string& name);
			static UniqueResource* Find(const std::filesystem::path& path);


			static std::size_t Count(const Resource* resource);
			static std::size_t Count(const UniqueResource* resource);


			static const std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual>& EntitiesAndComponents();
			static const std::unordered_set<Behavior*>& Behaviors();
			static const std::unordered_set<Component*>& Components(Entity* entity);
			static DirectionalLight* DirectionalLight();
			static const std::unordered_set<const SpotLight*>& SpotLights();
			static const std::vector<PointLight*>& PointLights();
			static const std::list<ShadowMap>& ShadowMaps();
			static const std::unordered_set<const ModelResource*>& Models();
			static const std::unordered_set<const ResourceHandleBase*>& ModelComponents(const ModelResource* model);


			static void DirectionalLight(leopph::DirectionalLight* dirLight);

			static void StoreMatrices(const Transform* transform, const Matrix4& model, const Matrix4& normal);
			static void DiscardMatrices(const Transform* transform);
			static const std::pair<Matrix4, Matrix4>& GetMatrices(const Transform* transform);


		private:
			static std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual> s_EntitiesAndComponents;

			static std::unordered_set<Behavior*> s_Behaviors;

			static leopph::DirectionalLight* s_DirLight;

			static std::unordered_set<const SpotLight*> s_SpotLights;

			static std::vector<PointLight*> s_PointLights;

			static std::unordered_map<const Transform*, std::pair<Matrix4, Matrix4>> s_Matrices;

			static std::unordered_set<const ModelResource*> s_ModelResources;

			static std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> s_ResourcesAndHandles;

			static std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> s_UniqueResourcesAndHandles;
	};
}