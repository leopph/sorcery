#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../components/rendering/InstancedRenderComponent.hpp"
#include "../components/rendering/NonInstancedRenderComponent.hpp"
#include "../components/rendering/RenderComponent.hpp"
#include "../entity/Entity.hpp"
#include "../rendering/Texture.hpp"
#include "../rendering/Skybox.hpp"
#include "../rendering/SkyboxImpl.hpp"
#include "../rendering/geometry/FileModelData.hpp"
#include "../rendering/geometry/InstancedRenderable.hpp"
#include "../rendering/geometry/InstancedRenderable.hpp"
#include "../rendering/geometry/ModelData.hpp"
#include "../rendering/geometry/NonInstancedRenderable.hpp"
#include "../util/equal/EntityEqual.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/equal/PointerEqual.hpp"
#include "../util/equal/RenderableEqual.hpp"
#include "../util/hash/EntityHash.hpp"
#include "../util/hash/PathedHash.hpp"
#include "../util/hash/PointerHash.hpp"
#include "../util/hash/RenderableHash.hpp"

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

		// Returns (and creates if non-existent) the InstancedRenderable that sources its data from the given ModelData object.
		static InstancedRenderable& CreateOrGetInstancedRenderable(ModelData& modelData);
		// Also unregisters all InstancedRenderComponents.
		static void DestroyInstancedRenderable(const InstancedRenderable& renderable);
		static void RegisterInstancedRenderComponent(const InstancedRenderable& renderable, InstancedRenderComponent* const component);
		static void UnregisterInstancedRenderComponent(const InstancedRenderable& renderable, InstancedRenderComponent* const component);
		static const std::unordered_map<InstancedRenderable, std::unordered_set<InstancedRenderComponent*>, RenderableHash, RenderableEqual>& InstancedRenderables();

		static void RegisterTexture(Texture* texture);
		static void UnregisterTexture(Texture* texture);
		static std::shared_ptr<Texture> FindTexture(const std::filesystem::path& path);

		static SkyboxImpl* CreateOrGetSkyboxImpl(std::filesystem::path allPaths);
		// Also unregisters all Skybox handles.
		static void DestroySkyboxImpl(SkyboxImpl* skybox);
		static void RegisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static void UnregisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static const std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>>& Skyboxes();

		static FileModelData& LoadOrGetFileModelData(std::filesystem::path path);

		// Creates a new NonInstancedRenderable instance, stores the referring NonInstancedRenderComponent with it, and returns a reference to it.
		static NonInstancedRenderable& CreateNonInstancedRenderable(ModelData& modelData, NonInstancedRenderComponent* const component);
		// Also unregister the NonInstancedRenderComponent.
		static void DestroyNonInstancedRenderable(const NonInstancedRenderable& renderable);
		static const std::unordered_map<std::unique_ptr<NonInstancedRenderable>, NonInstancedRenderComponent*, PointerHash, PointerEqual>& NonInstancedRenderables();


		static void Register(Entity* entity);
		static void Register(Behavior* behavior);
		static void Register(Component* component);
		static void Register(PointLight* pointLight);
		static void Register(const SpotLight* spotLight);

		static void Unregister(Entity* entity);
		static void Unregister(Behavior* behavior);
		static void Unregister(Component* component);
		static void Unregister(PointLight* pointLight);
		static void Unregister(const SpotLight* spotLight);

		static Entity* Find(const std::string& name);

		static const std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual>& EntitiesAndComponents();
		static const std::unordered_set<Behavior*>& Behaviors();
		static const std::unordered_set<Component*>& Components(Entity* entity);
		static DirectionalLight* DirectionalLight();
		static const std::unordered_set<const SpotLight*>& SpotLights();
		static const std::vector<PointLight*>& PointLights();

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

		// Stores non-owning pointers to all Texture instances.
		static std::vector<Texture*> s_Textures;

		// Stores SkyboxImpl instances along with all the Skybox handles pointing to it.
		static std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> s_Skyboxes;

		// Stores FileModelData instances identified by their file paths.
		static std::unordered_set<FileModelData, PathedHash<FileModelData>, PathedEqual<FileModelData>> s_FileModelData;

		// Stores all InstancedRenderable instances along with non-owning pointers to all the InstancedRenderComponents pointing to it.
		static std::unordered_map<InstancedRenderable, std::unordered_set<InstancedRenderComponent*>, RenderableHash, RenderableEqual> s_InstancedRenderables;

		// Stores unique_ptrs to all NonInstancedRenderable instances along with a non-owning pointer to the InstancedRenderComponent pointing to it.
		static std::unordered_map<std::unique_ptr<NonInstancedRenderable>, NonInstancedRenderComponent*, PointerHash, PointerEqual> s_NonInstancedRenderables;

		static void SortTextures();
	};
}
