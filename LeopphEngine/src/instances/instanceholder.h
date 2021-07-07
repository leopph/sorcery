#pragma once

#include "../components/lighting/pointlight.h"
#include "../components/lighting/dirlight.h"
#include "../hierarchy/object.h"
#include "../util/skybox.h"
#include "../util/skyboximpl.h"
#include "../rendering/texture.h"

#include "../util/objectcomparator.h"
#include "../util/modelreference.h"
#include "../util/textureequal.h"
#include "../util/texturehash.h"
#include "../util/texturereference.h"

#include <cstddef>
#include <functional>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/*------------------------------------------------------------
std::hash<std::filesystem::path> must be visible to s_Models*/
#include "../util/pathhash.h"
/*----------------------------------------------------------*/

namespace leopph::impl
{
	class InstanceHolder
	{
	public:
		static void DestroyAll();

		static const std::map<Object*, std::set<Component*>, ObjectComparator>& Objects();
		static void AddObject(Object* object);
		static void RemoveObject(Object* object);
		static Object* FindObject(const std::string& name);
		
		static bool IsTextureStored(const std::filesystem::path& path);
		static void StoreTexture(const Texture& other);
		static std::unique_ptr<Texture> GetTexture(const std::filesystem::path& path);
		static void AddTexture(const std::filesystem::path& path);
		static void RemoveTexture(const std::filesystem::path& path);

		static const std::set<Behavior*>& Behaviors();
		static void AddBehavior(Behavior* behavior);
		static void RemoveBehavior(Behavior* behavior);

		static const std::set<Component*>& Components(Object* object);
		static void AddComponent(Component* component);
		static void RemoveComponent(Component* component);

		static DirectionalLight* DirectionalLight();
		static void DirectionalLight(leopph::DirectionalLight* dirLight);

		static const std::vector<PointLight*>& PointLights();
		static void AddPointLight(PointLight* pointLight);
		static void RemovePointLight(PointLight* pointLight);

		static const AssimpModelImpl& GetModelReference(const std::filesystem::path& path);
		static void RegisterModelObject(const std::filesystem::path& path, Object* object);
		static void UnregisterModelObject(const std::filesystem::path& path, Object* object);

		static const std::unordered_map<std::filesystem::path, ModelReference>& Models();

		static std::size_t MeshCount(unsigned id);
		static void AddMesh(unsigned id);
		static void RemoveMesh(unsigned id);

		static const SkyboxImpl* GetSkybox(const std::filesystem::path& left, const std::filesystem::path& right,
			const std::filesystem::path& top, const std::filesystem::path& bottom,
			const std::filesystem::path& back, const std::filesystem::path& front);

		static const SkyboxImpl& GetSkybox(const Skybox& skybox);

		static const SkyboxImpl* RegisterSkybox(const std::filesystem::path& left, const std::filesystem::path& right,
			const std::filesystem::path& top, const std::filesystem::path& bottom,
			const std::filesystem::path& back, const std::filesystem::path& front);
		static void IncSkybox(const SkyboxImpl* skybox);
		static void DecSkybox(const SkyboxImpl* skybox);

		

	private:
		static std::unordered_set<TextureReference, TextureHash, TextureEqual> s_Textures;
		static std::unordered_map<unsigned, std::size_t> s_MeshCounts;
		static std::unordered_map<std::filesystem::path, ModelReference> s_Models;
		static std::unordered_map<SkyboxImpl, std::size_t, SkyboxImplHash, SkyboxImplEqual> s_Skyboxes;

		static std::set<Behavior*> s_Behaviors;
		static std::map<Object*, std::set<Component*>, ObjectComparator> s_Objects;

		static leopph::DirectionalLight* s_DirLight;
		static std::vector<PointLight*> s_PointLights;
	};
}