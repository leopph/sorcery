#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/lighting/AmbientLight.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../hierarchy/Object.hpp"
#include "../misc/skybox.h"
#include "../rendering/texture.h"
#include "../rendering/ShadowMap.hpp"
#include "skyboximpl.h"

#include "../util/equal/SkyBoxImplEqual.hpp"
#include "../util/equal/TextureEqual.hpp"
#include "../util/hash/PathHash.hpp"
#include "../util/hash/SkyBoxImplHash.hpp"
#include "../util/hash/TextureHash.hpp"
#include "../util/less/ObjectLess.hpp"
#include "modelreference.h"
#include "texturereference.h"

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
	class InstanceHolder
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
		
		/* Whether a TextureRef with given path is in the set */
		static bool IsTextureStored(const std::filesystem::path& path);
		/* Create a TextureRef from a new Texture and set count to 1 */
		static void StoreTextureRef(const Texture& other);
		/* Create a Texture from TextureRef and return it */
		static std::unique_ptr<Texture> CreateTexture(const std::filesystem::path& path);
		/* Inc count on TextureRef with give path */
		static void IncTexture(const std::filesystem::path& path);
		/* Dec count on TextureRef with give path */
		static void DecTexture(const std::filesystem::path& path);

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

		/* All ModelRefs */
		static const std::unordered_map<std::filesystem::path, ModelReference, PathHash>& Models();
		/* Reference to ModelRef on the given path */
		static const AssimpModelImpl& GetModelReference(const std::filesystem::path& path);
		/* Inc count of ModelRef on given path with given Object */
		static void IncModel(const std::filesystem::path& path, Object* object);
		/* Dec count of ModelRef on given path with given Object */
		static void DecModel(const std::filesystem::path& path, Object* object);

		/* Return pointer to SkyboxImpl on the given path, or nullptr, if not loaded yet */
		static const SkyboxImpl* GetSkybox(const std::filesystem::path& left, const std::filesystem::path& right,
			const std::filesystem::path& top, const std::filesystem::path& bottom,
			const std::filesystem::path& back, const std::filesystem::path& front);
		/* Return reference to SkyboxImpl based on the given Skybox's file names */
		static const SkyboxImpl& GetSkybox(const Skybox& skybox);
		/* Create a new SkyboxImpl on the given path and return a pointer to it */
		static const SkyboxImpl* RegisterSkybox(const std::filesystem::path& left, const std::filesystem::path& right,
			const std::filesystem::path& top, const std::filesystem::path& bottom,
			const std::filesystem::path& back, const std::filesystem::path& front);
		/* Inc count of given SkyboxImpl */
		static void IncSkybox(const SkyboxImpl* skybox);
		/* Dec count of given SkyboxImpl */
		static void DecSkybox(const SkyboxImpl* skybox);

		static const std::pair<const Matrix4, const Matrix4>& ModelAndNormalMatrices(const Object* object);

		static const std::list<ShadowMap>& ShadowMaps();
		static void CreateShadowMap(const Vector2& resolution);
		/* Removes the last stored Shadow Map */
		static void DeleteShadowMap();

		static const std::unordered_set<const SpotLight*>& SpotLights();
		static void RegisterSpotLight(const SpotLight* spotLight);
		static void UnregisterSpotLight(const SpotLight* spotLight);

	private:
		static std::unordered_set<TextureReference, TextureHash, TextureEqual> s_Textures;
		static std::unordered_map<std::filesystem::path, ModelReference, PathHash> s_Models;
		static std::unordered_map<SkyboxImpl, std::size_t, SkyboxImplHash, SkyboxImplEqual> s_Skyboxes;

		static std::set<Behavior*> s_Behaviors;
		static std::map<Object*, std::set<Component*>, ObjectLess> s_Objects;
		static std::unordered_map<const Object*, std::pair<const Matrix4, const Matrix4>> s_MatrixCache;

		static std::unique_ptr<leopph::AmbientLight> s_AmbientLight;
		static leopph::DirectionalLight* s_DirLight;
		static std::vector<PointLight*> s_PointLights;
		static std::unordered_set<const SpotLight*> s_SpotLights;

		static std::list<ShadowMap> s_ShadowMaps;
	};
}