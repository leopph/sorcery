#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../components/rendering/RenderComponent.hpp"
#include "../entity/Entity.hpp"
#include "../rendering/Skybox.hpp"
#include "../rendering/SkyboxImpl.hpp"
#include "../rendering/Texture.hpp"
#include "../rendering/geometry/GlMeshGroup.hpp"
#include "../rendering/geometry/MeshDataGroup.hpp"
#include "../util/equal/EntityEqual.hpp"
#include "../util/equal/GlMeshGroupEqual.hpp"
#include "../util/equal/IdEqual.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/equal/PointerEqual.hpp"
#include "../util/hash/EntityHash.hpp"
#include "../util/hash/GlMeshGroupHash.hpp"
#include "../util/hash/IdHash.hpp"
#include "../util/hash/PathedHash.hpp"
#include "../util/hash/PointerHash.hpp"

#include <filesystem>
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

		static void RegisterTexture(Texture* texture);
		static void UnregisterTexture(Texture* texture);
		static std::shared_ptr<Texture> FindTexture(const std::filesystem::path& path);

		static SkyboxImpl* CreateOrGetSkyboxImpl(std::filesystem::path allPaths);
		// Also unregisters all Skybox handles.
		static void DestroySkyboxImpl(SkyboxImpl* skybox);
		static void RegisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static void UnregisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static const std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>>& Skyboxes();

		static void RegisterComponentForEntity(const Entity* entity, std::unique_ptr<Component>&& component);
		// Destroys the Component object.
		static void UnregisterComponentFromEntity(const Entity* entity, const Component* component);
		static const std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>& ComponentsOfEntity(const Entity* entity);

		static void Register(Entity* entity);
		static void Register(Behavior* behavior);
		static void Register(PointLight* pointLight);
		static void Register(const SpotLight* spotLight);

		static void Unregister(Entity* entity);
		static void Unregister(Behavior* behavior);
		static void Unregister(PointLight* pointLight);
		static void Unregister(const SpotLight* spotLight);

		static Entity* Find(const std::string& name);

		static const std::unordered_map<Entity*, std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>, EntityHash, EntityEqual>& EntitiesAndComponents();
		static const std::unordered_set<Behavior*>& Behaviors();
		static DirectionalLight* DirectionalLight();
		static const std::unordered_set<const SpotLight*>& SpotLights();
		static const std::vector<PointLight*>& PointLights();
		static void DirectionalLight(leopph::DirectionalLight* dirLight);

		static void StoreMeshDataGroup(const MeshDataGroup& meshData);
		static const MeshDataGroup* FindMeshDataGroup(const std::string& id);

		/* Returns a copy of the stored GlMeshGroup that sources its data from the passed MeshDataGroup.
		 * If no instance is found, the function creates a new one. */
		static GlMeshGroup CreateOrGetMeshGroup(const MeshDataGroup& meshDataGroup);
		static void RegisterInstanceForMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance);
		static void UnregisterInstanceFromMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance);
		static std::unordered_map<GlMeshGroup, std::unordered_set<RenderComponent*>, GlMeshGroupHash, GlMeshGroupEqual>& MeshGroupsAndInstances();


	private:
		// Stores owning pointers to all Entities and the Components attached to them.
		static std::unordered_map<Entity*, std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>, EntityHash, EntityEqual> s_EntitiesAndComponents;

		static std::unordered_set<Behavior*> s_Behaviors;
		static leopph::DirectionalLight* s_DirLight;
		static std::unordered_set<const SpotLight*> s_SpotLights;
		static std::vector<PointLight*> s_PointLights;

		// Stores non-owning pointers to all Texture instances.
		static std::vector<Texture*> s_Textures;

		// Stores SkyboxImpl instances along with all the Skybox handles pointing to it.
		static std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> s_Skyboxes;

		static std::unordered_set<MeshDataGroup, IdHash, IdEqual> s_MeshData;

		static std::unordered_map<GlMeshGroup, std::unordered_set<RenderComponent*>, GlMeshGroupHash, GlMeshGroupEqual> s_Renderables;

		static void SortTextures();
	};
}
