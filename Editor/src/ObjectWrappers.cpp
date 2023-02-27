#include "ObjectWrappers.hpp"

#include <format>
#include <functional>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "ManagedRuntime.hpp"
#include "Systems.hpp"


#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ObjectFactoryManager.hpp"

namespace leopph::editor {
auto EditorObjectWrapperFor<BehaviorComponent>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto const& behavior{ dynamic_cast<BehaviorComponent&>(object) };

	auto const guidStr{ behavior.GetGuid().ToString() };
	if (ImGui::BeginTable(std::format("{}", guidStr).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		auto constexpr drawComponentMemberWidget = [](std::string_view const memberName, MonoType* const memberType, std::function<void*()> const& getFunc, std::function<void(void**)> const& setFunc) {
			std::string_view const memberTypeName = mono_type_get_name(memberType);
			auto const memberClass = mono_type_get_class(memberType);

			ImGui::TableNextRow();
			if (ImGui::TableGetRowIndex() == 0) {
				ImGui::TableSetColumnIndex(0);
				ImGui::PushItemWidth(FLT_MIN);
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(-FLT_MIN);
			}

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", memberName.data());
			ImGui::TableSetColumnIndex(1);

			auto const widgetLabel = std::format("##WidgetForMember{}", memberName);

			if (memberClass && mono_class_is_enum(memberClass)) {
				auto const enumValues = leopph::gManagedRuntime.GetEnumValues(mono_type_get_object(leopph::gManagedRuntime.GetManagedDomain(), memberType));
				auto const numEnumValues = mono_array_length(enumValues);
				int valueAlign;
				auto const valueSize = mono_type_size(mono_class_enum_basetype(memberClass), &valueAlign);

				auto const pCurrentValueUnboxed = getFunc();
				auto const currentValueBoxed = mono_value_box(leopph::gManagedRuntime.GetManagedDomain(), memberClass, pCurrentValueUnboxed);
				auto const currentValueManagedStr = mono_object_to_string(currentValueBoxed, nullptr);
				auto const currentValueStr = mono_string_to_utf8(currentValueManagedStr);

				if (ImGui::BeginCombo(widgetLabel.c_str(), currentValueStr)) {
					for (std::size_t i{ 0 }; i < numEnumValues; i++) {
						auto pValue = mono_array_addr_with_size(enumValues, valueSize, i);
						auto const valueBoxed = mono_value_box(leopph::gManagedRuntime.GetManagedDomain(), memberClass, reinterpret_cast<void*>(pValue));

						bool selected{ true };
						for (int j{ 0 }; j < valueSize; j++) {
							if (*static_cast<char*>(pCurrentValueUnboxed) != *pValue) {
								selected = false;
								break;
							}
						}

						if (ImGui::Selectable(mono_string_to_utf8(mono_object_to_string(valueBoxed, nullptr)), selected)) {
							setFunc(reinterpret_cast<void**>(&pValue));
						}
					}

					ImGui::EndCombo();
				}
			}
			else if (memberTypeName == "leopph.Vector3") {
				float data[3];
				std::memcpy(data, getFunc(), sizeof(data));
				if (ImGui::DragFloat3(widgetLabel.c_str(), data, 0.1f)) {
					auto pData = &data[0];
					setFunc(reinterpret_cast<void**>(&pData));
				}
			}
			else if (memberTypeName == "leopph.Quaternion") {
				auto euler = static_cast<leopph::Quaternion*>(getFunc())->ToEulerAngles();
				if (ImGui::DragFloat3(widgetLabel.c_str(), euler.GetData())) {
					auto quaternion = leopph::Quaternion::FromEulerAngles(euler[0], euler[1], euler[2]);
					auto pQuaternion = &quaternion;
					setFunc(reinterpret_cast<void**>(&pQuaternion));
				}
			}
			else if (memberTypeName == "System.Single") {
				float data;
				std::memcpy(&data, getFunc(), sizeof(data));
				if (ImGui::DragFloat(widgetLabel.c_str(), &data)) {
					auto pData = &data;
					setFunc(reinterpret_cast<void**>(&pData));
				}
			}
		};

		auto const obj{ behavior.GetManagedObject() };
		auto const klass{ mono_object_get_class(obj) };

		void* iter{ nullptr };
		while (auto const field = mono_class_get_fields(klass, &iter)) {
			auto const refField = mono_field_get_object(leopph::gManagedRuntime.GetManagedDomain(), klass, field);

			if (leopph::gManagedRuntime.ShouldSerialize(refField)) {
				drawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj] {
					                          return mono_object_unbox(mono_field_get_value_object(leopph::gManagedRuntime.GetManagedDomain(), field, obj));
				                          }, [field, obj](void** data) {
					                          mono_field_set_value(obj, field, *data);
				                          });
			}
		}

		iter = nullptr;

		while (auto const prop = mono_class_get_properties(klass, &iter)) {
			auto const refProp = mono_property_get_object(leopph::gManagedRuntime.GetManagedDomain(), klass, prop);

			if (leopph::gManagedRuntime.ShouldSerialize(refProp)) {
				drawComponentMemberWidget(mono_property_get_name(prop), mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop))), [prop, obj] {
					                          return mono_object_unbox(mono_property_get_value(prop, obj, nullptr, nullptr));
				                          }, [prop, obj](void** data) {
					                          mono_property_set_value(prop, reinterpret_cast<void*>(obj), data, nullptr);
				                          });
			}
		}

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<CameraComponent>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto& cam{ dynamic_cast<CameraComponent&>(object) };

	auto const guidStr{ cam.GetGuid().ToString() };
	if (ImGui::BeginTable(std::format("{}", guidStr).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Type");
		ImGui::TableNextColumn();

		char const* const typeOptions[]{ "Perspective", "Orthographic" };
		int selection{ cam.GetType() == CameraComponent::Type::Perspective ? 0 : 1 };
		if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
			cam.SetType(selection == 0 ? CameraComponent::Type::Perspective : CameraComponent::Type::Orthographic);
		}

		ImGui::TableNextColumn();

		if (cam.GetType() == CameraComponent::Type::Perspective) {
			ImGui::Text("Field Of View");
			ImGui::TableNextColumn();
			float value{ cam.GetPerspectiveFov() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "FOV").c_str(), &value)) {
				cam.SetPerspectiveFov(value);
			}
		}
		else {
			ImGui::Text("Size");
			ImGui::TableNextColumn();
			float value{ cam.GetOrthographicSize() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "OrthoSize").c_str(), &value)) {
				cam.SetOrthoGraphicSize(value);
			}
		}

		ImGui::TableNextColumn();
		ImGui::Text("Near Clip Plane");
		ImGui::TableNextColumn();

		float nearValue{ cam.GetNearClipPlane() };
		if (ImGui::DragFloat(std::format("{}{}", guidStr, "NearClip").c_str(), &nearValue)) {
			cam.SetNearClipPlane(nearValue);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Far Clip Plane");
		ImGui::TableNextColumn();

		float farValue{ cam.GetFarClipPlane() };
		if (ImGui::DragFloat(std::format("{}{}", guidStr, "FarClip").c_str(), &farValue)) {
			cam.SetFarClipPlane(farValue);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Viewport");
		ImGui::TableNextColumn();

		auto viewport{ cam.GetViewport() };
		if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportX").c_str(), &viewport.position.x)) {
			cam.SetViewport(viewport);
		}
		if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportY").c_str(), &viewport.position.y)) {
			cam.SetViewport(viewport);
		}
		if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportW").c_str(), &viewport.extent.width)) {
			cam.SetViewport(viewport);
		}
		if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportH").c_str(), &viewport.extent.height)) {
			cam.SetViewport(viewport);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Background Color");
		ImGui::TableNextColumn();

		Vector4 color{ cam.GetBackgroundColor() };
		if (ImGui::ColorEdit4("###backgroundColor", color.GetData())) {
			cam.SetBackgroundColor(color);
		}

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<CubeModelComponent>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto& model{ dynamic_cast<CubeModelComponent&>(object) };

	if (ImGui::BeginTable(std::format("{}", model.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Material");
		ImGui::TableNextColumn();

		static std::vector<Material*> materials;
		static std::string matFilter;

		if (ImGui::Button("Select##SelectMaterialForCubeModel")) {
			Object::FindObjectsOfType(materials);
			matFilter.clear();
			ImGui::OpenPopup("ChooseMaterialForCubeModel");
		}

		if (ImGui::BeginPopup("ChooseMaterialForCubeModel")) {
			if (ImGui::InputText("###SearchMat", &matFilter)) {
				Object::FindObjectsOfType(materials);
				std::erase_if(materials, [](Material const* mat) {
					return !mat->GetName().contains(matFilter);
				});
			}

			for (auto const mat : materials) {
				if (ImGui::Selectable(std::format("{}##matoption{}", mat->GetName(), mat->GetGuid().ToString()).c_str())) {
					model.SetMaterial(*mat);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::Text("%s", model.GetMaterial().GetName().data());

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Mesh");
		ImGui::TableNextColumn();

		static std::vector<Mesh*> meshes;
		static std::string meshFilter;

		if (ImGui::Button("Select##SelectMeshForStaticMeshComponent")) {
			Object::FindObjectsOfType(meshes);
			meshFilter.clear();
			ImGui::OpenPopup("ChooseMeshForStaticMeshComponent");
		}

		if (ImGui::BeginPopup("ChooseMeshForStaticMeshComponent")) {
			if (ImGui::InputText("###SearchMesh", &meshFilter)) {
				Object::FindObjectsOfType(meshes);
				std::erase_if(meshes, [](Mesh const* mesh) {
					return !mesh->GetName().contains(meshFilter);
				});
			}

			for (auto const mesh : meshes) {
				if (ImGui::Selectable(std::format("{}##meshoption{}", mesh->GetName(), mesh->GetGuid().ToString()).c_str())) {
					model.SetMesh(*mesh);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();

		ImGui::Text("%s", model.GetMesh().GetName().data());

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<Entity>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void {
	auto& entity{ dynamic_cast<Entity&>(object) };

	static std::string entityName;
	entityName = entity.GetName();

	if (ImGui::BeginTable("Property Widgets", 2)) {
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::Text("Name");

		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);
		if (ImGui::InputText("##EntityName", &entityName)) {
			entity.SetName(entityName);
		}

		ImGui::EndTable();
	}

	for (static std::vector<leopph::Component*> components; auto const& component : entity.GetComponents(components)) {
		auto const obj = component->GetManagedObject();
		auto const klass = mono_object_get_class(obj);

		auto const componentNodeId = mono_class_get_name(klass);
		if (ImGui::TreeNodeEx(componentNodeId, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Separator();
			objectFactoryManager.GetFor(component->GetSerializationType()).OnGui(objectFactoryManager, *component);
			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextItem(componentNodeId)) {
			if (ImGui::MenuItem("Delete")) {
				entity.DestroyComponent(component);
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
				entity.CreateComponent(componentClass);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

auto EditorObjectWrapperFor<LightComponent>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto& light{ dynamic_cast<LightComponent&>(object) };

	if (ImGui::BeginTable(std::format("{}", light.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Color");
		ImGui::TableNextColumn();

		Vector3 color{ light.GetColor() };
		if (ImGui::ColorEdit3("###lightColor", color.GetData())) {
			light.SetColor(color);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Intensity");
		ImGui::TableNextColumn();

		auto intensity{ light.GetIntensity() };
		if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f)) {
			light.SetIntensity(intensity);
		}

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<Material>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto& mat{ dynamic_cast<Material&>(object) };

	if (ImGui::BeginTable(std::format("{}", mat.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Albedo Color");
		ImGui::TableNextColumn();

		if (Vector3 albedoColor{ mat.GetAlbedoVector() }; ImGui::ColorEdit3("###matAlbedoColor", albedoColor.GetData())) {
			mat.SetAlbedoVector(albedoColor);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Metallic");
		ImGui::TableNextColumn();

		if (f32 metallic{ mat.GetMetallic() }; ImGui::SliderFloat("###matMetallic", &metallic, 0.0f, 1.0f)) {
			mat.SetMetallic(metallic);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Roughness");
		ImGui::TableNextColumn();

		if (f32 roughness{ mat.GetRoughness() }; ImGui::SliderFloat("###matRoughness", &roughness, 0.0f, 1.0f)) {
			mat.SetRoughness(roughness);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Ambient Occlusion");
		ImGui::TableNextColumn();

		if (f32 ao{ mat.GetAo() }; ImGui::SliderFloat("###matAo", &ao, 0.0f, 1.0f)) {
			mat.SetAo(ao);
		}

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<TransformComponent>::OnGui(EditorObjectFactoryManager const&, Object& object) -> void {
	auto& transform{ dynamic_cast<TransformComponent&>(object) };

	if (ImGui::BeginTable(std::format("{}", transform.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Local Position");
		ImGui::TableNextColumn();

		Vector3 localPos{ transform.GetLocalPosition() };
		if (ImGui::DragFloat3("###transformPos", localPos.GetData(), 0.1f)) {
			transform.SetLocalPosition(localPos);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Local Rotation");
		ImGui::TableNextColumn();

		auto euler{ transform.GetLocalRotation().ToEulerAngles() };
		if (ImGui::DragFloat3("###transformRot", euler.GetData(), 1.0f)) {
			transform.SetLocalRotation(Quaternion::FromEulerAngles(euler));
		}

		ImGui::TableNextColumn();
		ImGui::Text("Local Scale");
		ImGui::TableNextColumn();

		bool static uniformScale{ true };
		auto constexpr scaleSpeed{ 0.01f };

		ImGui::Text("%s", "Uniform");
		ImGui::SameLine();
		ImGui::Checkbox("##UniformScaleCheck", &uniformScale);
		ImGui::SameLine();

		if (uniformScale) {
			f32 scale{ transform.GetLocalScale()[0] };
			if (ImGui::DragFloat("###transformScale", &scale, scaleSpeed)) {
				transform.SetLocalScale(Vector3{ scale });
			}
		}
		else {
			Vector3 localScale{ transform.GetLocalScale() };
			if (ImGui::DragFloat3("###transformScale", localScale.GetData(), scaleSpeed)) {
				transform.SetLocalScale(localScale);
			}
		}

		ImGui::EndTable();
	}
}

auto EditorObjectWrapperFor<Entity>::Instantiate() -> Object* {
	return nullptr; // TODO
}
}
