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
#include "../util/equal/GlMeshGroupEqual.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/hash/GlMeshGroupHash.hpp"
#include "../util/hash/PathedHash.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	class DataManager
	{
		public:
			[[nodiscard]] static auto Instance() -> DataManager&;

			auto Clear() -> void;

			auto RegisterTexture(Texture* texture) -> void;
			auto UnregisterTexture(Texture* texture) -> void;
			[[nodiscard]] auto FindTexture(const std::filesystem::path& path) -> std::shared_ptr<Texture>;

			[[nodiscard]] auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;
			[[nodiscard]] auto SkyboxHandleCount(const SkyboxImpl* skybox) const -> std::size_t;
			// Unregisters all Skybox handles and destroys the impl instance.
			auto DestroySkyboxImpl(const SkyboxImpl* skybox) -> void;
			auto RegisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;
			auto UnregisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Takes ownership of the Entity and stores it.
			auto StoreEntity(std::unique_ptr<Entity> entity) -> void;
			// Removes the registered entry, and destroys the Entity and its Components.
			auto DestroyEntity(const Entity* entity) -> void;
			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]] auto FindEntity(const std::string& name) -> Entity*;
			[[nodiscard]] auto ComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&;
			// Adds the Component to the list of Components for the Entity.
			auto RegisterComponentForEntity(const Entity* entity, std::unique_ptr<Component>&& component) -> void;
			// Destroys the Component object.
			auto UnregisterComponentFromEntity(const Entity* entity, const Component* component) -> void;

			auto RegisterBehavior(Behavior* behavior) -> void;
			auto UnregisterBehavior(const Behavior* behavior) -> void;

			auto RegisterSpotLight(const SpotLight* spotLight) -> void;
			auto UnregisterSpotLight(const SpotLight* spotLight) -> void;

			auto RegisterPointLight(const PointLight* pointLight) -> void;
			auto UnregisterPointLight(const PointLight* pointLight) -> void;

			auto RegisterMeshDataGroup(MeshDataGroup* meshData) -> void;
			auto UnregisterMeshDataGroup(MeshDataGroup* meshData) -> void;

			/* Returns a copy of the stored GlMeshGroup that sources its data from the passed MeshDataGroup.
			 * If no instance is found, the function creates a new one. */
			[[nodiscard]] auto CreateOrGetMeshGroup(std::shared_ptr<const MeshDataGroup>&& meshDataGroup) -> const GlMeshGroup*;
			[[nodiscard]] auto FindMeshDataGroup(const std::string& id) -> std::shared_ptr<MeshDataGroup>;
			[[nodiscard]] auto MeshGroupInstanceCount(const GlMeshGroup& meshGroup) const -> std::size_t;
			// Unregisters all instances and destroys the MeshGroup
			auto DestroyMeshGroup(const GlMeshGroup* meshGroup) -> void;
			auto RegisterInstanceForMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void;
			auto UnregisterInstanceFromMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void;

			[[nodiscard]] constexpr auto Behaviors() const noexcept -> auto&;
			[[nodiscard]] constexpr auto DirectionalLight() const noexcept;
			[[nodiscard]] constexpr auto SpotLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto PointLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto MeshGroupsAndInstances() const noexcept -> auto&;

			constexpr auto DirectionalLight(const leopph::DirectionalLight* dirLight) noexcept;

		private:
			struct EntityAndComponents
			{
				// Owning pointer to Entity
				std::unique_ptr<Entity> Entity;
				// Owning pointers to Components
				std::vector<std::unique_ptr<Component>> Components;
			};


			// Non-owning pointers to all MeshDataGroup instances.
			std::vector<MeshDataGroup*> m_MeshData;

			// Non-owning pointers to all Texture instances.
			std::vector<Texture*> m_Textures;

			// Non-owning pointer to the lastly created DirectionalLight.
			const leopph::DirectionalLight* m_DirLight{nullptr};

			// Non-owning pointers to all SpotLights.
			std::vector<const SpotLight*> m_SpotLights;

			// Non-owning pointers to all PointLights.
			std::vector<const PointLight*> m_PointLights;

			// Non-owning pointers to all Behaviors.
			std::vector<Behavior*> m_Behaviors;

			// SkyboxImpl instances and non-owning pointer to all the Skybox handles pointing to them.
			std::unordered_map<SkyboxImpl, std::vector<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> m_Skyboxes;

			// GlMeshGroup instances and non-owning pointers to RenderComponents pointing to them.
			std::unordered_map<GlMeshGroup, std::vector<RenderComponent*>, GlMeshGroupHash, GlMeshGroupEqual> m_Renderables;

			// All Entities and all of their attached Components
			std::vector<EntityAndComponents> m_EntitiesAndComponents;

			auto SortTextures() -> void;
			auto SortEntities() -> void;
			auto SortMeshData() -> void;

			// Return a non-const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) -> decltype(m_EntitiesAndComponents)::iterator;
			// Return a const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) const -> decltype(m_EntitiesAndComponents)::const_iterator;
			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static auto FindEntityInternalCommon(auto* self, const std::string& name) -> decltype(auto);
	};


	[[nodiscard]] constexpr auto DataManager::Behaviors() const noexcept -> auto&
	{
		return m_Behaviors;
	}


	[[nodiscard]] constexpr auto DataManager::DirectionalLight() const noexcept
	{
		return m_DirLight;
	}


	[[nodiscard]] constexpr auto DataManager::SpotLights() const noexcept -> auto&
	{
		return m_SpotLights;
	}


	[[nodiscard]] constexpr auto DataManager::PointLights() const noexcept -> auto&
	{
		return m_PointLights;
	}


	[[nodiscard]] constexpr auto DataManager::MeshGroupsAndInstances() const noexcept -> auto&
	{
		return m_Renderables;
	}


	constexpr auto DataManager::DirectionalLight(const leopph::DirectionalLight* const dirLight) noexcept
	{
		m_DirLight = dirLight;
	}


	[[nodiscard]] auto DataManager::FindEntityInternalCommon(auto* const self, const std::string& name) -> decltype(auto)
	{
		const auto it{
			std::ranges::lower_bound(self->m_EntitiesAndComponents, name, [](const auto& elemName, const auto& value)
			                         {
				                         return elemName < value;
			                         }, [](const auto& elem) -> const auto&
			                         {
				                         return elem.Entity->Name();
			                         })
		};
		if (it != self->m_EntitiesAndComponents.end() && it->Entity->Name() == name)
		{
			return it;
		}
		return decltype(it){self->m_EntitiesAndComponents.end()};
	}
}
