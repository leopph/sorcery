#include "Entity.hpp"

#include "../data/DataManager.hpp"
#include "../util/logger.h"


namespace leopph
{
	auto Entity::CreateEntity(std::string name) -> Entity*
	{
		if (!NameIsUnused(name))
		{
			name = GenerateUnusedName(name);
			internal::Logger::Instance().Warning("Name collision detected. Entity is being renamed to " + name + ".");
		}
		std::unique_ptr<Entity> entity{new Entity{std::move(name)}};
		const auto ret{entity.get()};
		internal::DataManager::Instance().StoreEntity(std::move(entity));
		ret->RegisterComponent(std::make_unique<leopph::Transform>(ret));
		return ret;
	}


	auto Entity::DestroyEntity(const Entity* const entity) -> void
	{
		internal::DataManager::Instance().DestroyEntity(entity);
	}


	auto Entity::FindEntity(const std::string& name) -> Entity*
	{
		return internal::DataManager::Instance().FindEntity(name);
	}


	Entity::Entity(std::string name) :
		m_Name{std::move(name)},
		m_TransformCache{nullptr}
	{}


	auto Entity::RegisterComponent(std::unique_ptr<Component>&& component) const -> void
	{
		internal::DataManager::Instance().RegisterComponentForEntity(std::move(component));
	}


	auto Entity::RemoveComponent(const Component* component) const -> void
	{
		if (component->Entity() != this)
		{
			const auto msg{"Error while trying to remove Component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component)) + "] from Entity [" + m_Name + "]. The Component's owning Entity is different from this."};
			internal::Logger::Instance().Error(msg);
			return;
		}
		if (component == Transform())
		{
			const auto msg{"Transform component cannot be removed from Entity."};
			internal::Logger::Instance().Error(msg);
			return;
		}
		internal::DataManager::Instance().UnregisterComponentFromEntity(component);
	}


	auto Entity::Transform() const -> leopph::Transform*
	{
		if (!m_TransformCache)
		{
			m_TransformCache = GetComponent<leopph::Transform>();
		}
		return m_TransformCache;
	}


	auto Entity::Components() const -> const std::vector<std::unique_ptr<Component>>&
	{
		return internal::DataManager::Instance().ComponentsOfEntity(this);
	}


	auto Entity::GenerateUnusedName(const std::string& namePrefix) -> std::string
	{
		static std::size_t entityCounter{0};
		auto newName{namePrefix + std::to_string(entityCounter)};
		++entityCounter;
		while (!NameIsUnused(newName))
		{
			newName = namePrefix + std::to_string(entityCounter);
			++entityCounter;
		}
		return newName;
	}


	auto Entity::NameIsUnused(const std::string& name) -> bool
	{
		return !internal::DataManager::Instance().FindEntity(name);
	}
}
