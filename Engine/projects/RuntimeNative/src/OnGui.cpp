#include "OnGui.hpp"
#include "Components.hpp"

#include "ManagedRuntime.hpp"

#include <imgui.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <format>
#include <functional>

namespace leopph {
	auto SetImGuiContext(ImGuiContext* context) -> void {
		ImGui::SetCurrentContext(context);
	}


	auto Transform::OnGui() -> void {
		if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(FLT_MIN);
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-FLT_MIN);
			
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Local Position");
			ImGui::TableNextColumn();

			Vector3 localPos{ mLocalPosition };
			if (ImGui::DragFloat3("###transformPos", localPos.get_data(), 0.1f)) {
				SetLocalPosition(localPos);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Local Rotation");
			ImGui::TableNextColumn();

			auto euler{ mLocalRotation.ToEulerAngles() };
			if (ImGui::DragFloat3("###transformRot", euler.get_data(), 1.0f)) {
				SetLocalRotation(Quaternion::FromEulerAngles(euler));
			}

			ImGui::TableNextColumn();
			ImGui::Text("Local Scale");
			ImGui::TableNextColumn();

			Vector3 localScale{ mLocalScale };
			if (ImGui::DragFloat3("###transformScale", localScale.get_data(), 0.1f)) {
				SetLocalScale(localScale);
			}

			ImGui::EndTable();
		}
	}


	auto Camera::OnGui() -> void {
		auto const guidStr{ GetGuid().ToString() };
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
			int selection{ mType == Type::Perspective ? 0 : 1 };
			if (ImGui::Combo("###CameraType", &selection, typeOptions, 2)) {
				SetType(selection == 0 ? Type::Perspective : Type::Orthographic);
			}

			ImGui::TableNextColumn();

			if (mType == Type::Perspective) {
				ImGui::Text("Field Of View");
				ImGui::TableNextColumn();
				float value{ GetPerspectiveFov() };
				if (ImGui::DragFloat(std::format("{}{}", guidStr, "FOV").c_str(), &value)) {
					SetPerspectiveFov(value);
				}
			}
			else {
				ImGui::Text("Size");
				ImGui::TableNextColumn();
				float value{ GetOrthographicSize() };
				if (ImGui::DragFloat(std::format("{}{}", guidStr, "OrthoSize").c_str(), &value)) {
					SetOrthoGraphicSize(value);
				}
			}

			ImGui::TableNextColumn();
			ImGui::Text("Near Clip Plane");
			ImGui::TableNextColumn();

			float nearValue{ GetNearClipPlane() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "NearClip").c_str(), &nearValue)) {
				SetNearClipPlane(nearValue);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Far Clip Plane");
			ImGui::TableNextColumn();

			float farValue{ GetFarClipPlane() };
			if (ImGui::DragFloat(std::format("{}{}", guidStr, "FarClip").c_str(), &farValue)) {
				SetFarClipPlane(farValue);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Viewport");
			ImGui::TableNextColumn();

			auto viewport{ GetViewport() };
			if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportX").c_str(), &viewport.position.x)) {
				SetViewport(viewport);
			}
			if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportY").c_str(), &viewport.position.y)) {
				SetViewport(viewport);
			}
			if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportW").c_str(), &viewport.extent.width)) {
				SetViewport(viewport);
			}
			if (ImGui::InputFloat(std::format("{}{}", guidStr, "ViewportH").c_str(), &viewport.extent.height)) {
				SetViewport(viewport);
			}

			ImGui::EndTable();
		}
	}


	auto Behavior::OnGui() -> void {
		auto const guidStr{ GetGuid().ToString() };
		if (ImGui::BeginTable(std::format("{}", guidStr).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
			auto constexpr drawComponentMemberWidget = [](std::string_view const memberName, MonoType* const memberType, std::function<void* ()> const& getFunc, std::function<void(void**)> const& setFunc) {
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
				ImGui::Text(memberName.data());
				ImGui::TableSetColumnIndex(1);

				auto const widgetLabel = std::format("##WidgetForMember{}", memberName);

				if (memberClass && mono_class_is_enum(memberClass)) {
					auto const enumValues = leopph::GetEnumValues(mono_type_get_object(leopph::GetManagedDomain(), memberType));
					auto const numEnumValues = mono_array_length(enumValues);
					int valueAlign;
					auto const valueSize = mono_type_size(mono_class_enum_basetype(memberClass), &valueAlign);

					auto const pCurrentValueUnboxed = getFunc();
					auto const currentValueBoxed = mono_value_box(leopph::GetManagedDomain(), memberClass, pCurrentValueUnboxed);
					auto const currentValueManagedStr = mono_object_to_string(currentValueBoxed, nullptr);
					auto const currentValueStr = mono_string_to_utf8(currentValueManagedStr);

					if (ImGui::BeginCombo(widgetLabel.c_str(), currentValueStr)) {
						for (std::size_t i{ 0 }; i < numEnumValues; i++) {
							auto pValue = mono_array_addr_with_size(enumValues, valueSize, i);
							auto const valueBoxed = mono_value_box(leopph::GetManagedDomain(), memberClass, reinterpret_cast<void*>(pValue));

							bool selected{ true };
							for (std::size_t j{ 0 }; j < valueSize; j++) {
								if (*reinterpret_cast<char*>(pCurrentValueUnboxed) != *pValue) {
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
					auto euler = reinterpret_cast<leopph::Quaternion*>(getFunc())->ToEulerAngles();
					if (ImGui::DragFloat3(widgetLabel.c_str(), euler.get_data())) {
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

			auto const obj{ GetManagedObject() };
			auto const klass{ mono_object_get_class(obj) };

			void* iter{ nullptr };
			while (auto const field = mono_class_get_fields(klass, &iter)) {
				auto const refField = mono_field_get_object(leopph::GetManagedDomain(), klass, field);

				if (leopph::ShouldSerialize(refField)) {
					drawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj] {
						return mono_object_unbox(mono_field_get_value_object(leopph::GetManagedDomain(), field, obj));
					}, [field, obj](void** data) {
						mono_field_set_value(obj, field, *data);
					});
				}
			}

			iter = nullptr;

			while (auto const prop = mono_class_get_properties(klass, &iter)) {
				auto const refProp = mono_property_get_object(leopph::GetManagedDomain(), klass, prop);

				if (leopph::ShouldSerialize(refProp)) {
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


	auto CubeModel::OnGui() -> void {

	}


	auto Light::OnGui() -> void {
		if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(FLT_MIN);
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-FLT_MIN);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Color");
			ImGui::TableNextColumn();

			Vector3 color{ mColor };
			if (ImGui::ColorEdit3("###lightColor", color.get_data())) {
				SetColor(color);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Intensity");
			ImGui::TableNextColumn();

			auto intensity{ mIntensity };
			if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f)) {
				SetIntensity(intensity);
			}

			ImGui::EndTable();
		}
	}
}