#include "Entity.hpp"

#include "Components.hpp"
#include "SceneManager.hpp"
#include "ManagedRuntime.hpp"
#include "Systems.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/object.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <functional>


namespace leopph {
	namespace {
		template<std::derived_from<Component> T>
		std::unique_ptr<Component> instantiate() {
			return std::make_unique<T>();
		}


		std::unordered_map<std::string_view, std::function<std::unique_ptr<Component>()>> const gComponentInstantiators
		{
			{"CubeModel", instantiate<CubeModel>},
			{"Camera", instantiate<Camera>},
			{"DirectionalLight", instantiate<DirectionalLight>}
		};
	}

	std::vector<Entity*> Entity::sAllEntities;


	auto Entity::GetAllEntities() -> std::span<Entity* const> {
		return sAllEntities;
	}


	auto Entity::GetAllEntities(std::vector<Entity*>& outEntities) -> std::vector<Entity*>& {
		outEntities.clear();
		for (auto* const entity : sAllEntities) {
			outEntities.emplace_back(entity);
		}
		return outEntities;
	}


	auto Entity::FindEntityByName(std::string_view name) -> Entity* {
		for (auto* const entity : sAllEntities) {
			if (entity->mName == name) {
				return entity;
			}
		}
		return nullptr;
	}

	Entity::Entity() {
		sAllEntities.emplace_back(this);
	}


	Entity::~Entity() {
		std::erase(sAllEntities, this);
	}


	auto Entity::GetScene() const -> Scene& {
		return *mScene;
	}


	auto Entity::GetTransform() const -> Transform& {
		if (!mTransform) {
			mTransform = GetComponent<Transform>();
		}
		return *mTransform;
	}


	auto Entity::CreateComponent(MonoClass* const componentClass) -> Component*
	{
		if (mono_class_is_subclass_of(componentClass, mono_class_from_name(gManagedRuntime.GetManagedImage(), "leopph", "Behavior"), false)) {
			auto behavior{ std::make_unique<Behavior>(componentClass) };
			behavior->CreateManagedObject(componentClass);
			auto const ret{ behavior.get()};
			AddComponent(std::move(behavior));
			return ret;
		}

		std::string_view const className{ mono_class_get_name(componentClass) };

		if (className == "Transform") {
			return &GetTransform();
		}

		if (auto const it{ gComponentInstantiators.find(className) }; it != std::end(gComponentInstantiators)) {
			auto component{ it->second() };
			component->CreateManagedObject(componentClass);
			auto const ret{ component.get()};
			AddComponent(std::move(component));
			return ret;
		}

		return nullptr;
	}


	auto Entity::AddComponent(std::unique_ptr<Component> component) -> void {
		if (component) {
			component->SetEntity(this);
			mComponents.push_back(std::move(component));
		}
	}


	auto Entity::DestroyComponent(Component* const component) -> void {
		if (component) {
			if (component->GetEntity()->GetGuid() != GetGuid() || component->GetTransform().GetGuid() == component->GetGuid()) {
				return;
			}
			std::erase_if(mComponents, [component](auto const& attachedComponent) {
				return attachedComponent->GetGuid() == component->GetGuid();
			});
		}
	}

	void Entity::OnGui() {
		ManagedAccessObject::OnGui();

		static std::string entityName;
		entityName = GetName();

		if (ImGui::BeginTable("Property Widgets", 2)) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(FLT_MIN);
			ImGui::Text("Name");

			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-FLT_MIN);
			if (ImGui::InputText("##EntityName", &entityName)) {
				SetName(entityName);
			}

			ImGui::EndTable();
		}

		for (static std::vector<leopph::Component*> components; auto const& component : GetComponents(components)) {
			auto const obj = component->GetManagedObject();
			auto const klass = mono_object_get_class(obj);

			auto const componentNodeId = mono_class_get_name(klass);
			if (ImGui::TreeNodeEx(componentNodeId, ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Separator();
				component->OnGui();
				ImGui::TreePop();
			}

			if (ImGui::BeginPopupContextItem(componentNodeId)) {
				if (ImGui::MenuItem("Delete")) {
					DestroyComponent(component);
				}
				ImGui::EndPopup();
			}
			ImGui::OpenPopupOnItemClick(componentNodeId, ImGuiPopupFlags_MouseButtonRight);
		}

		auto constexpr addNewComponentLabel = "Add New Component";
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(addNewComponentLabel).x) * 0.5f);
		ImGui::Button(addNewComponentLabel);

		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
			for (auto const& componentClass : leopph::gManagedRuntime.GetComponentClasses()) {
				auto const componentName = mono_class_get_name(componentClass);
				if (ImGui::MenuItem(componentName)) {
					CreateComponent(componentClass);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}
	}


	namespace managedbindings {
		void CreateNativeEntity(MonoObject* managedEntity) {
			auto const entity = SceneManager::GetActiveScene()->CreateEntity();
			entity->SetManagedObject(managedEntity);

			auto transform = std::make_unique<Transform>();
			transform->CreateManagedObject("leopph", "Transform");

			entity->AddComponent(std::move(transform));
		}


		MonoObject* GetEntityTransform(MonoObject* managedEntity) {
			auto const nativeEntity = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity);
			return nativeEntity->GetTransform().GetManagedObject();
		}


		MonoString* GetEntityName(MonoObject* managedEntity) {
			return mono_string_new_wrapper(ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->GetName().data());
		}


		void SetEntityName(MonoObject* managedEntity, MonoString* managedName) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->SetName(mono_string_to_utf8(managedName));
		}


		MonoObject* EntityCreateComponent(Entity* const entity, MonoReflectionType* const componentType) {
			auto const componentClass{ mono_type_get_class(mono_reflection_type_get_type(componentType)) };
			auto const component{ entity->CreateComponent(componentClass) };
			return component ? component->GetManagedObject() : nullptr;
		}
	}
}
