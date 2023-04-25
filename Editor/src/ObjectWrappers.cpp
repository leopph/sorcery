#include "ObjectWrappers.hpp"

#include <format>
#include <functional>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ManagedRuntime.hpp"
#include "Renderer.hpp"
#include "Systems.hpp"
#include "CubemapImporter.hpp"
#include "EditorContext.hpp"
#include "ObjectFactoryManager.hpp"
#include "Texture2DImporter.hpp"
#include "MeshImporter.hpp"
#include "MaterialImporter.hpp"
#include "SceneImporter.hpp"
#include "Util.hpp"


namespace leopph::editor {
auto EditorObjectWrapperFor<BehaviorComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
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
				auto const enumValues = gManagedRuntime.GetEnumValues(mono_type_get_object(gManagedRuntime.GetManagedDomain(), memberType));
				auto const numEnumValues = mono_array_length(enumValues);
				int valueAlign;
				auto const valueSize = mono_type_size(mono_class_enum_basetype(memberClass), &valueAlign);

				auto const pCurrentValueUnboxed = getFunc();
				auto const currentValueBoxed = mono_value_box(gManagedRuntime.GetManagedDomain(), memberClass, pCurrentValueUnboxed);
				auto const currentValueManagedStr = mono_object_to_string(currentValueBoxed, nullptr);
				auto const currentValueStr = mono_string_to_utf8(currentValueManagedStr);

				if (ImGui::BeginCombo(widgetLabel.c_str(), currentValueStr)) {
					for (std::size_t i{ 0 }; i < numEnumValues; i++) {
						auto pValue = mono_array_addr_with_size(enumValues, valueSize, i);
						auto const valueBoxed = mono_value_box(gManagedRuntime.GetManagedDomain(), memberClass, reinterpret_cast<void*>(pValue));

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
				auto euler = static_cast<Quaternion*>(getFunc())->ToEulerAngles();
				if (ImGui::DragFloat3(widgetLabel.c_str(), euler.GetData())) {
					auto quaternion = Quaternion::FromEulerAngles(euler[0], euler[1], euler[2]);
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
			auto const refField = mono_field_get_object(gManagedRuntime.GetManagedDomain(), klass, field);

			if (gManagedRuntime.ShouldSerialize(refField)) {
				drawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj] {
					                          return mono_object_unbox(mono_field_get_value_object(gManagedRuntime.GetManagedDomain(), field, obj));
				                          }, [field, obj](void** data) {
					                          mono_field_set_value(obj, field, *data);
				                          });
			}
		}

		iter = nullptr;

		while (auto const prop = mono_class_get_properties(klass, &iter)) {
			auto const refProp = mono_property_get_object(gManagedRuntime.GetManagedDomain(), klass, prop);

			if (gManagedRuntime.ShouldSerialize(refProp)) {
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


auto EditorObjectWrapperFor<CameraComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
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
		int selection{ cam.GetType() == renderer::Camera::Type::Perspective ? 0 : 1 };
		if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
			cam.SetType(selection == 0 ? renderer::Camera::Type::Perspective : renderer::Camera::Type::Orthographic);
		}

		ImGui::TableNextColumn();

		if (cam.GetType() == renderer::Camera::Type::Perspective) {
			ImGui::Text("Field Of View");
			ImGui::TableNextColumn();
			float value{ cam.GetHorizontalPerspectiveFov() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "FOV").c_str(), &value)) {
				cam.SetHorizontalPerspectiveFov(value);
			}
		}
		else {
			ImGui::Text("Size");
			ImGui::TableNextColumn();
			float value{ cam.GetHorizontalOrthographicSize() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "OrthoSize").c_str(), &value)) {
				cam.SetHorizontalOrthographicSize(value);
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


auto EditorObjectWrapperFor<StaticMeshComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
	auto& model{ dynamic_cast<StaticMeshComponent&>(object) };

	if (ImGui::BeginTable(std::format("{}", model.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
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
					return !Contains(mesh->GetName(), meshFilter);
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

		ImGui::TableNextColumn();
		ImGui::Text("Material Count");
		ImGui::TableNextColumn();

		auto mtlCount{ clamp_cast<int>(model.GetMaterials().size()) };
		if (ImGui::InputInt("###MtlCountInput", &mtlCount) && mtlCount >= 0) {
			while (mtlCount > clamp_cast<int>(model.GetMaterials().size())) {
				model.AddMaterial(*renderer::GetDefaultMaterial());
			}
			while (mtlCount < clamp_cast<int>(model.GetMaterials().size())) {
				model.RemoveMaterial(clamp_cast<int>(model.GetMaterials().size()) - 1);
			}
		}

		static std::vector<Material*> allMaterials;
		static std::string matFilter;

		if (mtlCount > 0) {
			ImGui::TableNextColumn();
			ImGui::Text("Materials");
			ImGui::TableNextColumn();

			for (int i = 0; i < mtlCount; i++) {
				ImGui::TableSetColumnIndex(1);

				auto const popupId{ std::format("ChooseMaterial{}ForCube", std::to_string(i)) };

				if (ImGui::Button(std::format("Select##SelectMaterial{}ForModel", std::to_string(i)).c_str())) {
					Object::FindObjectsOfType(allMaterials);
					matFilter.clear();
					ImGui::OpenPopup(popupId.c_str());
				}

				if (ImGui::BeginPopup(popupId.c_str())) {
					if (ImGui::InputText("###SearchMat", &matFilter)) {
						Object::FindObjectsOfType(allMaterials);
						std::erase_if(allMaterials, [](Material const* mat) {
							return !Contains(mat->GetName(), matFilter);
						});
					}

					for (auto const mat : allMaterials) {
						if (ImGui::Selectable(std::format("{}##matoption{}", mat->GetName(), mat->GetGuid().ToString()).c_str())) {
							model.ReplaceMaterial(i, *mat);
							break;
						}
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::Text("%s", model.GetMaterials()[i]->GetName().data());
				ImGui::TableNextRow();
			}
		}

		ImGui::EndTable();
	}
}


auto EditorObjectWrapperFor<Entity>::OnGui(Context& context, Object& object) -> void {
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

	for (static std::vector<Component*> components; auto const& component : entity.GetComponents(components)) {
		auto const obj = component->GetManagedObject();
		auto const klass = mono_object_get_class(obj);

		auto const componentNodeId = mono_class_get_name(klass);
		if (ImGui::TreeNodeEx(componentNodeId, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Separator();
			context.GetFactoryManager().GetFor(component->GetSerializationType()).OnGui(context, *component);
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
		for (auto const& componentClass : gManagedRuntime.GetComponentClasses()) {
			auto const componentName = mono_class_get_name(componentClass);
			if (ImGui::MenuItem(componentName)) {
				entity.CreateComponent(componentClass);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}


auto EditorObjectWrapperFor<LightComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
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
		if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f, LightComponent::MIN_INTENSITY, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
			light.SetIntensity(intensity);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Casts Shadow");
		ImGui::TableNextColumn();

		auto castsShadow{ light.IsCastingShadow() };
		if (ImGui::Checkbox("###lightCastsShadow", &castsShadow)) {
			light.SetCastingShadow(castsShadow);
		}

		if (light.IsCastingShadow()) {
			ImGui::TableNextColumn();

			if (light.GetType() == LightComponent::Type::Directional) {
				ImGui::Text("%s", "Shadow Extension");
				ImGui::TableNextColumn();

				float shadowExt{ light.GetShadowExtension() };
				if (ImGui::DragFloat("##lightShadowExt", &shadowExt, 1.0f, LightComponent::MIN_SHADOW_EXTENSION, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
					light.SetShadowExtension(shadowExt);
				}
			}
			else {
				ImGui::Text("%s", "Shadow Near Plane");
				ImGui::TableNextColumn();

				auto shadowNearPlane{ light.GetShadowNearPlane() };
				if (ImGui::DragFloat("###lightShadowNearPlane", &shadowNearPlane, 0.01f, LightComponent::MIN_SHADOW_NEAR_PLANE, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
					light.SetShadowNearPlane(shadowNearPlane);
				}
			}

			ImGui::TableNextColumn();
			ImGui::Text("%s", "Shadow Depth Bias");
			ImGui::TableNextColumn();

			auto shadowDepthBias{ light.GetShadowDepthBias() };
			if (ImGui::DragFloat("###lightShadowDephBias", &shadowDepthBias, 0.25f, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				light.SetShadowDepthBias(shadowDepthBias);
			}

			ImGui::TableNextColumn();
			ImGui::Text("%s", "Shadow Normal Bias");
			ImGui::TableNextColumn();

			auto shadowNormalBias{ light.GetShadowNormalBias() };
			if (ImGui::DragFloat("###lightShadowNormalBias", &shadowNormalBias, 0.25f, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				light.SetShadowNormalBias(shadowNormalBias);
			}
		}

		ImGui::TableNextColumn();
		ImGui::Text("Type");
		ImGui::TableNextColumn();

		constexpr char const* typeOptions[]{ "Directional", "Spot", "Point" };
		int selection{ static_cast<int>(light.GetType()) };
		if (ImGui::Combo("###LightType", &selection, typeOptions, 3)) {
			light.SetType(static_cast<LightComponent::Type>(selection));
		}

		if (light.GetType() == LightComponent::Type::Spot || light.GetType() == LightComponent::Type::Point) {
			ImGui::TableNextColumn();
			ImGui::Text("Range");
			ImGui::TableNextColumn();

			auto range{ light.GetRange() };
			if (ImGui::DragFloat("###lightRange", &range, 1.0f, LightComponent::MIN_RANGE, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				light.SetRange(range);
			}
		}

		if (light.GetType() == LightComponent::Type::Spot) {
			ImGui::TableNextColumn();
			ImGui::Text("Inner Angle");
			ImGui::TableNextColumn();

			auto innerAngleRad{ ToRadians(light.GetInnerAngle()) };
			if (ImGui::SliderAngle("###spotLightInnerAngle", &innerAngleRad, LightComponent::MIN_ANGLE_DEG, LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				light.SetInnerAngle(ToDegrees(innerAngleRad));
			}

			ImGui::TableNextColumn();
			ImGui::Text("Outer Angle");
			ImGui::TableNextColumn();

			auto outerAngleRad{ ToRadians(light.GetOuterAngle()) };
			if (ImGui::SliderAngle("###spotLightOuterAngle", &outerAngleRad, LightComponent::MIN_ANGLE_DEG, LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				light.SetOuterAngle(ToDegrees(outerAngleRad));
			}
		}

		ImGui::EndTable();
	}
}


auto EditorObjectWrapperFor<Material>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
	auto& mtl{ dynamic_cast<Material&>(object) };

	if (ImGui::BeginTable(std::format("{}", mtl.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Albedo Color");
		ImGui::TableNextColumn();

		if (Vector3 albedoColor{ mtl.GetAlbedoVector() }; ImGui::ColorEdit3("###matAlbedoColor", albedoColor.GetData())) {
			mtl.SetAlbedoVector(albedoColor);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Metallic");
		ImGui::TableNextColumn();

		if (f32 metallic{ mtl.GetMetallic() }; ImGui::SliderFloat("###matMetallic", &metallic, 0.0f, 1.0f)) {
			mtl.SetMetallic(metallic);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Roughness");
		ImGui::TableNextColumn();

		if (f32 roughness{ mtl.GetRoughness() }; ImGui::SliderFloat("###matRoughness", &roughness, 0.0f, 1.0f)) {
			mtl.SetRoughness(roughness);
		}

		ImGui::TableNextColumn();
		ImGui::Text("Ambient Occlusion");
		ImGui::TableNextColumn();

		if (f32 ao{ mtl.GetAo() }; ImGui::SliderFloat("###matAo", &ao, 0.0f, 1.0f)) {
			mtl.SetAo(ao);
		}

		static std::vector<Texture2D*> textures;
		static std::string texFilter;

		auto constexpr nullTexName{ "None" };

		std::function<Texture2D*()> static texGetFn;
		std::function<void(Texture2D*)> static texSetFn;

		auto constexpr popupId{ "SelectTexturePopUp" };

		if (ImGui::BeginPopup(popupId)) {
			if (ImGui::InputText("###SearchTextures", &texFilter)) {
				Object::FindObjectsOfType(textures);
				std::erase_if(textures, [](Texture2D const* tex) {
					return tex && !Contains(tex->GetName(), texFilter);
				});
			}

			for (auto const tex : textures) {
				if (ImGui::Selectable(std::format("{}##texoption{}", tex ? tex->GetName() : nullTexName, tex ? tex->GetGuid().ToString() : "0").c_str(), tex == texGetFn())) {
					texSetFn(tex);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Albedo Map");
		ImGui::TableNextColumn();

		if (ImGui::Button("Select##SelectAlbedoMapButton")) {
			Object::FindObjectsOfType(textures);
			textures.insert(std::begin(textures), nullptr);
			texFilter.clear();
			texGetFn = [&mtl] {
				return mtl.GetAlbedoMap();
			};
			texSetFn = [&mtl](Texture2D* const tex) {
				mtl.SetAlbedoMap(tex);
			};
			ImGui::OpenPopup(popupId);
		}

		ImGui::SameLine();
		ImGui::Text("%s", mtl.GetAlbedoMap() ? mtl.GetAlbedoMap()->GetName().data() : nullTexName);

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Metallic Map");
		ImGui::TableNextColumn();

		if (ImGui::Button("Select##SelectMetallicMapButton")) {
			Object::FindObjectsOfType(textures);
			textures.insert(std::begin(textures), nullptr);
			texFilter.clear();
			texGetFn = [&mtl] {
				return mtl.GetMetallicMap();
			};
			texSetFn = [&mtl](Texture2D* const tex) {
				mtl.SetMetallicMap(tex);
			};
			ImGui::OpenPopup(popupId);
		}

		ImGui::SameLine();
		ImGui::Text("%s", mtl.GetMetallicMap() ? mtl.GetMetallicMap()->GetName().data() : nullTexName);

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Roughness Map");
		ImGui::TableNextColumn();

		if (ImGui::Button("Select##SelectRoughnessMapButton")) {
			Object::FindObjectsOfType(textures);
			textures.insert(std::begin(textures), nullptr);
			texFilter.clear();
			texGetFn = [&mtl] {
				return mtl.GetRoughnessMap();
			};
			texSetFn = [&mtl](Texture2D* const tex) {
				mtl.SetRoughnessMap(tex);
			};
			ImGui::OpenPopup(popupId);
		}

		ImGui::SameLine();
		ImGui::Text("%s", mtl.GetRoughnessMap() ? mtl.GetRoughnessMap()->GetName().data() : nullTexName);

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Ambient Occlusion Map");
		ImGui::TableNextColumn();

		if (ImGui::Button("Select##SelectAoMapButton")) {
			Object::FindObjectsOfType(textures);
			textures.insert(std::begin(textures), nullptr);
			texFilter.clear();
			texGetFn = [&mtl] {
				return mtl.GetAoMap();
			};
			texSetFn = [&mtl](Texture2D* const tex) {
				mtl.SetAoMap(tex);
			};
			ImGui::OpenPopup(popupId);
		}

		ImGui::SameLine();
		ImGui::Text("%s", mtl.GetAoMap() ? mtl.GetAoMap()->GetName().data() : nullTexName);

		ImGui::EndTable();
	}

	if (ImGui::Button("Save##SaveMaterialAsset")) {
		context.SaveRegisteredNativeAsset(mtl);
	}
}


auto EditorObjectWrapperFor<TransformComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
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


auto EditorObjectWrapperFor<Mesh>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
	if (auto const& mesh{ dynamic_cast<Mesh&>(object) }; ImGui::BeginTable(std::format("{}", mesh.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", "Vertex Count");

		ImGui::TableNextColumn();
		ImGui::Text("%s", std::to_string(mesh.GetPositions().size()).c_str());

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Index Count");

		ImGui::TableNextColumn();
		ImGui::Text("%s", std::to_string(mesh.GetIndices().size()).c_str());

		ImGui::EndTable();
	}
}


auto EditorObjectWrapperFor<Texture2D>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
	auto const& tex{ dynamic_cast<Texture2D&>(object) };

	if (ImGui::BeginTable(std::format("{}", tex.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", "Width");

		ImGui::TableNextColumn();
		ImGui::Text("%s", std::to_string(tex.GetImageData().get_width()).c_str());

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Height");

		ImGui::TableNextColumn();
		ImGui::Text("%s", std::to_string(tex.GetImageData().get_height()).c_str());

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Channel Count");

		ImGui::TableNextColumn();
		ImGui::Text("%s", std::to_string(tex.GetImageData().get_num_channels()).c_str());

		ImGui::EndTable();
	}

	auto const contentRegion{ ImGui::GetContentRegionAvail() };
	auto const imgWidth{ static_cast<float>(tex.GetImageData().get_width()) };
	auto const imgHeight{ static_cast<float>(tex.GetImageData().get_height()) };
	auto const widthRatio{ contentRegion.x / imgWidth };
	auto const heightRatio{ contentRegion.y / imgHeight };
	ImVec2 displaySize;

	if (widthRatio > heightRatio) {
		displaySize.x = imgWidth * heightRatio;
		displaySize.y = imgHeight * heightRatio;
	}
	else {
		displaySize.x = imgWidth * widthRatio;
		displaySize.y = imgHeight * widthRatio;
	}

	ImGui::Image(tex.GetSrv(), displaySize);
}


auto EditorObjectWrapperFor<Scene>::OnGui(Context& context, Object& object) -> void {
	ImGui::Text("%s", "Scene Asset");
	if (ImGui::Button("Open")) {
		context.OpenScene(dynamic_cast<Scene&>(object));
	}
}


auto EditorObjectWrapperFor<SkyboxComponent>::OnGui([[maybe_unused]] Context& context, Object& object) -> void {
	auto& skybox{ dynamic_cast<SkyboxComponent&>(object) };

	ImGui::PushID(skybox.GetGuid().ToString().c_str());

	if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", "Cubemap");

		ImGui::TableNextColumn();
		static std::vector<Cubemap*> cubemaps;
		static std::string cubemapFilter;

		auto constexpr cubemapSelectPopupId{ "Select Cubemap" };
		auto constexpr cubemapNullName{ "None" };

		if (ImGui::Button("Select##Cubemap")) {
			Object::FindObjectsOfType(cubemaps);
			cubemaps.insert(std::begin(cubemaps), nullptr);
			cubemapFilter.clear();
			ImGui::OpenPopup(cubemapSelectPopupId);
		}

		if (ImGui::BeginPopup(cubemapSelectPopupId)) {
			if (ImGui::InputText("##Filter", &cubemapFilter)) {
				Object::FindObjectsOfType(cubemaps);
				std::erase_if(cubemaps, [](Cubemap const* cubemap) {
					return cubemap && !Contains(cubemap->GetName(), cubemapFilter);
				});
			}

			for (auto const cubemap : cubemaps) {
				ImGui::PushID(cubemap ? cubemap->GetGuid().ToString().c_str() : "Cubemap None Id");
				if (ImGui::Selectable(cubemap ? cubemap->GetName().data() : cubemapNullName)) {
					skybox.SetCubemap(cubemap);
				}
				ImGui::PopID();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::Text("%s", skybox.GetCubemap() ? skybox.GetCubemap()->GetName().data() : cubemapNullName);

		ImGui::EndTable();
	}

	ImGui::PopID();
}


auto EditorObjectWrapperFor<Entity>::Instantiate() -> Object* {
	return Entity::New();
}


auto EditorObjectWrapperFor<Mesh>::GetImporter() -> Importer& {
	MeshImporter static meshImporter;
	return meshImporter;
}


auto EditorObjectWrapperFor<Texture2D>::GetImporter() -> Importer& {
	Texture2DImporter static texImporter;
	return texImporter;
}


auto EditorObjectWrapperFor<Material>::GetImporter() -> Importer& {
	MaterialImporter static mtlImporter;
	return mtlImporter;
}


auto EditorObjectWrapperFor<Scene>::GetImporter() -> Importer& {
	SceneImporter static sceneImporter;
	return sceneImporter;
}


auto EditorObjectWrapperFor<Cubemap>::GetImporter() -> Importer& {
	CubemapImporter static cubemapImporter;
	return cubemapImporter;
}


auto EditorObjectWrapperFor<Entity>::OnDrawGizmosSelected(Context& context, Object& object) -> void {
	auto const& entity{ dynamic_cast<Entity&>(object) };
	std::vector<Component*> static components;
	components.clear();
	entity.GetComponents(components);

	for (auto const component : components) {
		context.GetFactoryManager().GetFor(component->GetSerializationType()).OnDrawGizmosSelected(context, *component);
	}
}


auto EditorObjectWrapperFor<LightComponent>::OnDrawGizmosSelected([[maybe_unused]] Context& context, Object& object) -> void {
	auto const& light{ dynamic_cast<LightComponent&>(object) };

	if (light.GetType() == LightComponent::Type::Spot) {
		auto const modelMtxNoScale{ CalculateModelMatrixNoScale(light.GetEntity()->GetTransform()) };
		auto vertices{ CalculateSpotLightLocalVertices(light) };

		for (auto& vertex : vertices) {
			vertex = Vector3{ Vector4{ vertex, 1 } * modelMtxNoScale };
		}

		Color const lineColor{ Color::Magenta() };

		// This highly depends on the order CalculateSpotLightLocalVertices returns the vertices
		for (int i = 0; i < 4; i++) {
			renderer::DrawLineAtNextRender(vertices[4], vertices[i], lineColor);
			renderer::DrawLineAtNextRender(vertices[i], vertices[(i + 1) % 4], lineColor);
		}
	}
}
}
