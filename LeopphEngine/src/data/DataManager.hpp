#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/InstancedModel.hpp"
#include "../components/Transform.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../entity/Entity.hpp"
#include "../util/equal/EntityEqual.hpp"
#include "../util/equal/InstancedModelImplEqual.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/hash/EntityHash.hpp"
#include "../util/hash/InstancedModelImplHash.hpp"
#include "../util/hash/PathedHash.hpp"
#include "../rendering/TextureImpl.hpp"
#include "../rendering/Texture.hpp"
#include "../rendering/Skybox.hpp"
#include "../rendering/SkyboxImpl.hpp"
#include "../rendering/geometry/FileModelData.hpp"
#include "../rendering/geometry/Renderable.hpp"
#include "../rendering/geometry/ModelData.hpp"

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

		// Returns (and creates if non-existent) the InstancedModelImpl that sources its data from the given ModelData object.
		static InstancedModelImpl& CreateOrGetInstancedModelImpl(ModelData& modelData);
		// Also unregisters all Model components.
		static void DestroyInstancedModelImpl(const InstancedModelImpl& model);
		static void RegisterModelComponent(const InstancedModelImpl& model, InstancedModel* component);
		static void UnregisterModelComponent(const InstancedModelImpl& model, InstancedModel* component);
		static const std::unordered_map<InstancedModelImpl, std::unordered_set<InstancedModel*>, InstancedModelImplHash, InstancedModelImplEqual>& InstancedModels();

		static TextureImpl* CreateOrGetTextureImpl(std::filesystem::path path);
		// Also unregisters all Texture handles.
		static void DestroyTextureImpl(TextureImpl* texture);
		static void RegisterTextureHandle(TextureImpl* texture, Texture* handle);
		static void UnregisterTextureHandle(TextureImpl* texture, Texture* handle);
		static const std::unordered_map<TextureImpl, std::unordered_set<Texture*>, PathedHash<TextureImpl>, PathedEqual<TextureImpl>>& Textures();

		static SkyboxImpl* CreateOrGetSkyboxImpl(std::filesystem::path allPaths);
		// Also unregisters all Skybox handles.
		static void DestroySkyboxImpl(SkyboxImpl* skybox);
		static void RegisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static void UnregisterSkyboxHandle(SkyboxImpl* skybox, Skybox* handle);
		static const std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>>& Skyboxes();

		static FileModelData& LoadOrGetFileModelData(std::filesystem::path path);

		static const std::unordered_set<const Renderable*>& Renderables();
		static void RegisterRenderable(const Renderable* renderable);
		static void UnregisterRenderable(const Renderable* renderable);


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
		// Stores TextureImpl instances along with all the Texture handles pointing to it.
		static std::unordered_map<TextureImpl, std::unordered_set<Texture*>, PathedHash<TextureImpl>, PathedEqual<TextureImpl>> s_Textures;
		// Stores SkyboxImpl instances along with all the Skybox handles pointing to it.
		static std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> s_Skyboxes;
		// Stores FileModelData instances identified by their file paths.
		static std::unordered_set<FileModelData, PathedHash<FileModelData>, PathedEqual<FileModelData>> s_FileModelData;
		// Stores non-owning pointers to all created Renderables. Does not manage lifetime.
		static std::unordered_set<const Renderable*> s_Renderables;
		// Stores all InstancedModelImpl instances along with all the InstancedModel components pointing to it.
		static std::unordered_map<InstancedModelImpl, std::unordered_set<InstancedModel*>, InstancedModelImplHash, InstancedModelImplEqual> s_InstancedModels;
	};
}
