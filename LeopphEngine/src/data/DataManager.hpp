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
#include "../util/equal/IdEqual.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/hash/GlMeshGroupHash.hpp"
#include "../util/hash/IdHash.hpp"
#include "../util/hash/PathedHash.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
			auto FindTexture(const std::filesystem::path& path) -> std::shared_ptr<Texture>;

			auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;
			// Also unregisters all Skybox handles.
			auto DestroySkyboxImpl(const SkyboxImpl* skybox) -> void;
			auto RegisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;
			auto UnregisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Takes ownership of the Entity and stores it.
			auto StoreEntity(std::unique_ptr<Entity> entity) -> void;
			// Removes the registered entry, and destroys the Entity and its Components.
			auto DestroyEntity(const Entity* entity) -> void;
			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			auto FindEntity(const std::string& name) -> Entity*;
			// Adds the Component to the list of Components for the Entity.
			auto RegisterComponentForEntity(const Entity* entity, std::unique_ptr<Component>&& component) -> void;
			// Destroys the Component object.
			auto UnregisterComponentFromEntity(const Entity* entity, const Component* component) -> void;
			[[nodiscard]]
			auto ComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&;

			auto RegisterBehavior(Behavior* behavior) -> void;
			auto UnregisterBehavior(Behavior* behavior) -> void;

			auto RegisterSpotLight(const SpotLight* spotLight) -> void;
			auto UnregisterSpotLight(const SpotLight* spotLight) -> void;

			auto RegisterPointLight(PointLight* pointLight) -> void;
			auto UnregisterPointLight(PointLight* pointLight) -> void;

			auto RegisterMeshDataGroup(MeshDataGroup* meshData) -> void;
			auto UnregisterMeshDataGroup(MeshDataGroup* meshData) -> void;
			[[nodiscard]]
			auto FindMeshDataGroup(const std::string& id) -> std::shared_ptr<MeshDataGroup>;

			/* Returns a copy of the stored GlMeshGroup that sources its data from the passed MeshDataGroup.
			 * If no instance is found, the function creates a new one. */
			[[nodiscard]]
			auto CreateOrGetMeshGroup(std::shared_ptr<const MeshDataGroup>&& meshDataGroup) -> const GlMeshGroup*;
			auto RegisterInstanceForMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void;
			// If the MeshGroup runs out of instances it is destroyed.
			auto UnregisterInstanceFromMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void;

			[[nodiscard]] constexpr auto EntitiesAndComponents() const noexcept -> auto&
			{
				return m_EntitiesAndComponents;
			}

			[[nodiscard]] constexpr auto Behaviors() const noexcept -> auto&
			{
				return m_Behaviors;
			}

			[[nodiscard]] constexpr auto DirectionalLight() const noexcept
			{
				return m_DirLight;
			}

			[[nodiscard]] constexpr auto SpotLights() const noexcept -> auto&
			{
				return m_SpotLights;
			}

			[[nodiscard]] constexpr auto PointLights() const noexcept -> auto&
			{
				return m_PointLights;
			}

			[[nodiscard]] constexpr auto Skyboxes() const noexcept -> auto&
			{
				return m_Skyboxes;
			}

			[[nodiscard]] constexpr auto MeshGroupsAndInstances() const noexcept -> auto&
			{
				return m_Renderables;
			}

			constexpr auto DirectionalLight(leopph::DirectionalLight* const dirLight) noexcept
			{
				m_DirLight = dirLight;
			}

		private:
			struct EntityAndComponents
			{
				// Owning pointer to Entity
				std::unique_ptr<Entity> Entity;
				// Owning pointers to Components
				std::vector<std::unique_ptr<Component>> Components;
			};


			// All Entities and all of their attached Components
			std::vector<EntityAndComponents> m_EntitiesAndComponents;

			// Non-owning pointers to all Behaviors.
			std::unordered_set<Behavior*> m_Behaviors;

			// Non-owning pointer to the lastly created DirectionalLight.
			leopph::DirectionalLight* m_DirLight{nullptr};

			// Non-owning pointers to all SpotLights.
			std::unordered_set<const SpotLight*> m_SpotLights;

			// Non-owning pointers to all PointLights.
			std::vector<PointLight*> m_PointLights;

			// Non-owning pointers to all Texture instances.
			std::vector<Texture*> m_Textures;

			// SkyboxImpl instances and non-owning pointer to all the Skybox handles pointing to them.
			std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> m_Skyboxes;

			// Non-owning pointers to all MeshDataGroup instances.
			std::unordered_set<MeshDataGroup*, IdHash, IdEqual> m_MeshData;

			// GlMeshGroup instances and non-owning pointers to RenderComponents pointing to them.
			std::unordered_map<GlMeshGroup, std::unordered_set<RenderComponent*>, GlMeshGroupHash, GlMeshGroupEqual> m_Renderables;

			auto SortTextures() -> void;
			auto SortEntities() -> void;

			// Return a non-const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) -> decltype(m_EntitiesAndComponents)::iterator;
			// Return a const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) const -> decltype(m_EntitiesAndComponents)::const_iterator;

			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static auto FindEntityInternalCommon(auto* const self, const std::string& name) -> decltype(auto)
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
	};
}
