#include "ObjectFactoryManager.hpp"


namespace leopph::editor {
auto CreateFactoryManager() noexcept -> EditorObjectFactoryManager {
	EditorObjectFactoryManager factoryManager;
	factoryManager.Register<Entity>();
	factoryManager.Register<TransformComponent>();
	factoryManager.Register<CameraComponent>();
	factoryManager.Register<BehaviorComponent>();
	factoryManager.Register<ModelComponent>();
	factoryManager.Register<LightComponent>();
	factoryManager.Register<Material>();
	factoryManager.Register<Mesh>();
	factoryManager.Register<Texture2D>();
	factoryManager.Register<Scene>();
	return factoryManager;
}
}
