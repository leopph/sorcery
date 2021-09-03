#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/lighting/AmbientLight.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../hierarchy/Object.hpp"
#include "../rendering/buffers/RefCountedBuffer.hpp"
#include "../rendering/geometry/ModelResource.hpp"
#include "../rendering/ShadowMap.hpp"
#include "managed/Resource.hpp"
#include "managed/ResourceHandleBase.hpp"
#include "managed/UniqueResource.hpp"

#include "../util/equal/UniqueResourceEqual.hpp"
#include "../util/hash/PathHash.hpp"
#include "../util/hash/UniqueResourceHash.hpp"
#include "../util/less/ObjectLess.hpp"

#include <cstddef>
#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <set>
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


		static void Register(Object* object);
		static void Register(Behavior* behavior);
		static void Register(Component* component);
		static void Register(PointLight* pointLight);
		static void Register(const ModelResource* model);
		static void Register(const Resource* resource);
		static void Register(const UniqueResource* resource);
		static void Register(const Resource* resource, const ResourceHandleBase* handle);
		static void Register(const UniqueResource* resource, const ResourceHandleBase* handle);
		static void Register(const SpotLight* spotLight);
		static void Register(const RefCountedBuffer& buffer);


		static void Unregister(Object* object);
		static void Unregister(Behavior* behavior);
		static void Unregister(Component* component);
		static void Unregister(PointLight* pointLight);
		static void Unregister(const ModelResource* model);
		static void Unregister(const Resource* resource);
		static void Unregister(const UniqueResource* resource);
		static void Unregister(const Resource* resource, const ResourceHandleBase* handle);
		static void Unregister(const UniqueResource* resource, const ResourceHandleBase* handle);
		static void Unregister(const SpotLight* spotLight);
		static void Unregister(const RefCountedBuffer& buffer);


		static Object* Find(const std::string& name);
		static UniqueResource* Find(const std::filesystem::path& path);


		static std::size_t Count(const RefCountedBuffer& buffer);
		static std::size_t Count(const Resource* resource);
		static std::size_t Count(const UniqueResource* resource);


		static const std::map<Object*, std::set<Component*>, ObjectLess>& Objects();
		static const std::set<Behavior*>& Behaviors();
		static const std::set<Component*>& Components(Object* object);
		static leopph::AmbientLight* AmbientLight();
		static DirectionalLight* DirectionalLight();
		static const std::unordered_set<const SpotLight*>& SpotLights();
		static const std::vector<PointLight*>& PointLights();
		static const std::pair<const Matrix4, const Matrix4>& ModelAndNormalMatrices(const Object* object);
		static const std::list<ShadowMap>& ShadowMaps();
		static const std::unordered_set<const ModelResource*> Models();
		static std::unordered_set<const ResourceHandleBase*> ModelComponents(const ModelResource* model);


		static void DirectionalLight(leopph::DirectionalLight* dirLight);
		static void AmbientLight(leopph::AmbientLight*&& light);
		static void CreateShadowMap(const Vector2& resolution);
		static void DeleteShadowMap();


	private:
		static std::map<Object*, std::set<Component*>, ObjectLess> s_Objects;

		static std::set<Behavior*> s_Behaviors;

		static std::unique_ptr<leopph::AmbientLight> s_AmbientLight;

		static leopph::DirectionalLight* s_DirLight;

		static std::unordered_set<const SpotLight*> s_SpotLights;

		static std::vector<PointLight*> s_PointLights;

		static std::unordered_map<const Object*, std::pair<const Matrix4, const Matrix4>> s_MatrixCache;

		static std::unordered_set<const ModelResource*> s_ModelResources;

		static std::list<ShadowMap> s_ShadowMaps;

		static std::unordered_map<unsigned, std::size_t> s_Buffers;

		static std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> s_ResourcesAndHandles;

		static std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> s_UniqueResourcesAndHandles;


	};
}