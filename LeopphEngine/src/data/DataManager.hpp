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
		/* All Objects*/
		static const std::map<Object*, std::set<Component*>, ObjectLess>& Objects();
		/* Store pointer to Object !!!OWNERSHIP!!! */
		static void RegisterObject(Object* object);
		/* Destruct Object and remove pointer */
		static void UnregisterObject(Object* object);
		/* Return pointer to Object based on name, or nullptr if it doesn't exist */
		static Object* FindObject(const std::string& name);
		/* Call destructor on all Objects */
		static void DestroyAllObjects();


		/* All Behaviors */
		static const std::set<Behavior*>& Behaviors();
		/* Add pointer of Behavior to registry */
		static void RegisterBehavior(Behavior* behavior);
		/* Remove pointer of Behavior from registry */
		static void UnregisterBehavior(Behavior* behavior);


		/* All Components attached to the given Object */
		static const std::set<Component*>& Components(Object* object);
		/* Add pointer of Component to registry */
		static void RegisterComponent(Component* component);
		/* Remove pointer of Component from registry */
		static void UnregisterComponent(Component* component);


		/* Current DirectionalLight */
		static DirectionalLight* DirectionalLight();
		/* Set DirectionalLight */
		static void DirectionalLight(leopph::DirectionalLight* dirLight);


		/* All PointLights */
		static const std::vector<PointLight*>& PointLights();
		/* Add pointer of PointLight to registry */
		static void RegisterPointLight(PointLight* pointLight);
		/* Remove pointer of PointLight from registry */
		static void UnregisterPointLight(PointLight* pointLight);


		/* AmbientLight instance */
		static leopph::AmbientLight* AmbientLight();
		/* Set the instance */
		static void AmbientLight(leopph::AmbientLight*&& light);


		static const std::pair<const Matrix4, const Matrix4>& ModelAndNormalMatrices(const Object* object);


		static const std::list<ShadowMap>& ShadowMaps();
		static void CreateShadowMap(const Vector2& resolution);
		/* Removes the last stored Shadow Map */
		static void DeleteShadowMap();


		static const std::unordered_set<const SpotLight*>& SpotLights();
		static void RegisterSpotLight(const SpotLight* spotLight);
		static void UnregisterSpotLight(const SpotLight* spotLight);


		static void RegisterBuffer(const RefCountedBuffer& buffer);
		static void UnregisterBuffer(const RefCountedBuffer& buffer);
		static std::size_t ReferenceCount(const RefCountedBuffer& buffer);


		static void RegisterResource(const Resource* resource);
		static void UnregisterResource(const Resource* resource);
		static void RegisterResource(const UniqueResource* resource);
		static void UnregisterResource(const UniqueResource* resource);
		static void RegisterResourceHandle(const Resource* resource, const ResourceHandleBase* handle);
		static void UnregisterResourceHandle(const Resource* resource, const ResourceHandleBase* handle);
		static void RegisterResourceHandle(const UniqueResource* resource, const ResourceHandleBase* handle);
		static void UnregisterResourceHandle(const UniqueResource* resource, const ResourceHandleBase* handle);
		static std::size_t ResourceHandleCount(const Resource* resource);
		static std::size_t ResourceHandleCount(const UniqueResource* resource);
		static UniqueResource* FindUniqueResource(const std::filesystem::path& path);


		static const std::unordered_set<const ModelResource*> Models();
		static void RegisterModel(const ModelResource* model);
		static void UnregisterModel(const ModelResource* model);
		static std::unordered_set<const ResourceHandleBase*> ModelComponents(const ModelResource* model);

	private:
		static std::set<Behavior*> s_Behaviors;
		static std::map<Object*, std::set<Component*>, ObjectLess> s_Objects;
		static std::unordered_map<const Object*, std::pair<const Matrix4, const Matrix4>> s_MatrixCache;

		static std::unique_ptr<leopph::AmbientLight> s_AmbientLight;
		static leopph::DirectionalLight* s_DirLight;
		static std::vector<PointLight*> s_PointLights;
		static std::unordered_set<const SpotLight*> s_SpotLights;

		static std::list<ShadowMap> s_ShadowMaps;

		static std::unordered_map<unsigned, std::size_t> s_Buffers;

		static std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> s_ResourcesAndHandles;
		static std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> s_UniqueResourcesAndHandles;

		static std::unordered_set<const ModelResource*> s_ModelResources;
	};
}