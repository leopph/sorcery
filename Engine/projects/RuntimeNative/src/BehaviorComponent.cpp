#include "BehaviorComponent.hpp"

#include <format>
#include <unordered_map>

#include <imgui.h>
#include <iostream>
#include <variant>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ManagedRuntime.hpp"
#include "Systems.hpp"

namespace leopph {
	namespace {
		std::unordered_map<BehaviorComponent*, MonoMethod*> gToInit;
		std::unordered_map<BehaviorComponent*, MonoMethod*> gToTick;
		std::unordered_map<BehaviorComponent*, MonoMethod*> gToTack;


		auto invoke_method_handle_exception(MonoObject* const obj, MonoMethod* const method) -> void {
			MonoObject* exception;
			mono_runtime_invoke(method, obj, nullptr, &exception);

			if (exception) {
				mono_print_unhandled_exception(exception);
			}
		}
	}


	BehaviorComponent::BehaviorComponent(MonoClass* const klass) {
		auto const obj = CreateManagedObject(klass);

		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0)) {
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0)) {
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0)) {
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0)) {
			invoke_method_handle_exception(obj, ctor);
		}
	}


	BehaviorComponent::~BehaviorComponent() {
		gToInit.erase(this);
		gToTick.erase(this);
		gToTack.erase(this);

		MonoObject* const managedObj = GetManagedObject();

		if (MonoMethod* const destroyMethod = mono_class_get_method_from_name(mono_object_get_class(managedObj), "OnDestroy", 0)) {
			invoke_method_handle_exception(managedObj, destroyMethod);
		}
	}


	auto BehaviorComponent::Init(MonoClass* klass) -> void {
		auto const obj = CreateManagedObject(klass);

		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0)) {
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0)) {
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0)) {
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0)) {
			invoke_method_handle_exception(obj, ctor);
		}
	}


	auto init_behaviors() -> void {
		for (auto const& [behavior, method] : gToInit) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}

		gToInit.clear();
	}


	auto tick_behaviors() -> void {
		for (auto const& [behavior, method] : gToTick) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	auto tack_behaviors() -> void {
		for (auto const& [behavior, method] : gToTack) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}

	auto BehaviorComponent::OnGui() -> void {
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
							for (std::size_t j{ 0 }; j < valueSize; j++) {
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

	auto BehaviorComponent::GetSerializationType() const -> Type {
		return Type::Behavior;
	}

	auto BehaviorComponent::SerializeTextual(YAML::Node& node) const -> void {
		Component::SerializeTextual(node);
		auto const componentClass = mono_object_get_class(GetManagedObject());
		node["classNameSpace"] = mono_class_get_namespace(componentClass);
		node["className"] = mono_class_get_name(componentClass);

		std::function<void(MonoObject*, YAML::Node&)> serializeObject;

		serializeObject = [&serializeObject](MonoObject* const obj, YAML::Node& node) -> void {
			auto const objClass = mono_object_get_class(obj);

			void* iter{ nullptr };

			while (auto const field = mono_class_get_fields(objClass, &iter)) {
				if (leopph::gManagedRuntime.ShouldSerialize(mono_field_get_object(leopph::gManagedRuntime.GetManagedDomain(), objClass, field))) {
					auto const fieldType = mono_field_get_type(field);
					auto const fieldRefType = mono_type_get_object(leopph::gManagedRuntime.GetManagedDomain(), fieldType);
					auto const fieldClass = mono_type_get_class(fieldType);
					auto const fieldName = mono_field_get_name(field);
					auto const fieldValueBoxed = mono_field_get_value_object(leopph::gManagedRuntime.GetManagedDomain(), field, obj);

					if (leopph::gManagedRuntime.IsTypePrimitive(fieldRefType)) {
						node[fieldName] = mono_string_to_utf8(mono_object_to_string(fieldValueBoxed, nullptr));
					}
					else if (mono_class_is_enum(fieldClass)) {
						node[fieldName] = mono_string_to_utf8(mono_object_to_string(leopph::gManagedRuntime.EnumToUnderlyingType(fieldRefType, fieldValueBoxed), nullptr));
					}
					else if (mono_class_is_valuetype(fieldClass)) {
						auto dataNode = node[fieldName];
						serializeObject(fieldValueBoxed, dataNode);
					}
					else {
						node[fieldName] = ManagedAccessObject::GetNativePtrFromManagedObjectAs<ManagedAccessObject*>(fieldValueBoxed)->GetGuid().ToString();
					}
				}
			}

			iter = nullptr;

			while (auto const prop = mono_class_get_properties(objClass, &iter)) {
				if (leopph::gManagedRuntime.ShouldSerialize(mono_property_get_object(leopph::gManagedRuntime.GetManagedDomain(), objClass, prop))) {
					auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
					auto const propRefType = mono_type_get_object(leopph::gManagedRuntime.GetManagedDomain(), propType);
					auto const propClass = mono_type_get_class(propType);
					auto const propName = mono_property_get_name(prop);
					auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
					auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);

					if (leopph::gManagedRuntime.IsTypePrimitive(propRefType)) {
						node[propName] = mono_string_to_utf8(mono_object_to_string(propValueBoxed, nullptr));
					}
					else if (mono_class_is_enum(propClass)) {
						node[propName] = mono_string_to_utf8(mono_object_to_string(leopph::gManagedRuntime.EnumToUnderlyingType(propRefType, propValueBoxed), nullptr));
					}
					else if (mono_class_is_valuetype(propClass)) {
						auto dataNode = node[propName];
						serializeObject(propValueBoxed, dataNode);
					}
					else {
						node[propName] = ManagedAccessObject::GetNativePtrFromManagedObjectAs<ManagedAccessObject*>(propValueBoxed)->GetGuid().ToString();
					}
				}
			}
		};

		serializeObject(GetManagedObject(), node);
	}


	auto BehaviorComponent::DeserializeTextual(YAML::Node const& node) -> void {
		auto const classNs = node["classNameSpace"].as<std::string>();
		auto const className = node["className"].as<std::string>();
		auto const componentClass = mono_class_from_name(leopph::gManagedRuntime.GetManagedImage(), classNs.c_str(), className.c_str());
		Init(componentClass);
		auto const managedComponent = GetManagedObject();

		std::function<void(MonoObject*, YAML::Node const&)> parseAndSetMembers;
		std::function<MonoObject* (YAML::Node const&, MonoObject*, std::variant<MonoProperty*, MonoClassField*>)> setMember;

		parseAndSetMembers = [&parseAndSetMembers, this](MonoObject* const obj, YAML::Node const& dataNode) -> void {
			auto const objClass = mono_object_get_class(obj);

			for (auto it = dataNode.begin(); it != dataNode.end(); ++it) {
				auto const memberName = it->first.as<std::string>();

				if (auto const prop = mono_class_get_property_from_name(objClass, memberName.data())) {
					auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
					auto const propRefType = mono_type_get_object(leopph::gManagedRuntime.GetManagedDomain(), propType);
					auto const propClass = mono_type_get_class(propType);
					auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
					auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);
					auto const refProp = mono_property_get_object(leopph::gManagedRuntime.GetManagedDomain(), objClass, prop);

					if (leopph::gManagedRuntime.IsTypePrimitive(propRefType)) {
						auto const memberValueStr = it->second.as<std::string>();
						auto const parsedMemberValueBoxed = leopph::gManagedRuntime.ParseValue(refProp, memberValueStr.c_str());
						if (!parsedMemberValueBoxed) {
							std::cerr << std::format("Failed to deserialize property {}::{} on BehaviorComponent {}. Invalid data.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString()) << std::endl;
						}
						else {
							auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
							mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
						}
					}
					else if (mono_class_is_enum(propClass)) {
						auto const memberValueStr = it->second.as<std::string>();
						auto const parsedMemberValueBoxed = leopph::gManagedRuntime.ParseEnumValue(propRefType, memberValueStr.c_str());
						if (!parsedMemberValueBoxed) {
							std::cerr << std::format("Failed to deserialize property {}::{} on BehaviorComponent {}. Invalid data.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString()) << std::endl;
						}
						else {
							auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
							mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
						}
					}
					else if (mono_class_is_valuetype(propClass)) {
						parseAndSetMembers(propValueBoxed, it->second);
						auto propValueUnboxed = mono_object_unbox(propValueBoxed);
						mono_property_set_value(prop, objPossiblyUnboxed, &propValueUnboxed, nullptr);
					}
					else {
						auto const guidStr{ it->second.as<std::string>() };
						auto const targetObj{ Object::FindObjectByGuid(Guid::Parse(guidStr)) };
						if (!targetObj) {
							std::cerr << std::format("Failed to deserialize property {}::{} on BehaviorComponent {}. Guid {} does not belong to any object.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString(), guidStr) << std::endl;
						}
						else {
							auto const targetObjCast{ dynamic_cast<ManagedAccessObject*>(targetObj) };
							if (!targetObjCast) {
								std::cerr << std::format("Failed to deserialize property {}::{} on BehaviorComponent {}. Object {} is not a LeopphEngine type. Currently only LeopphEngine types are supported for by-reference serialization.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString(), guidStr) << std::endl;
							}
							else {
								auto managedRefValue{ targetObjCast->GetManagedObject() };
								mono_property_set_value(prop, objPossiblyUnboxed, reinterpret_cast<void**>(&managedRefValue), nullptr);
							}
						}
					}
				}
				else if (auto const field = mono_class_get_field_from_name(objClass, memberName.data())) {
					auto const fieldType = mono_field_get_type(field);
					auto const fieldRefType = mono_type_get_object(leopph::gManagedRuntime.GetManagedDomain(), fieldType);
					auto const fieldClass = mono_type_get_class(fieldType);
					auto const refField = mono_field_get_object(leopph::gManagedRuntime.GetManagedDomain(), objClass, field);

					if (leopph::gManagedRuntime.IsTypePrimitive(fieldRefType)) {
						auto const memberValueStr = it->second.as<std::string>();
						auto const parsedMemberValueBoxed = leopph::gManagedRuntime.ParseValue(refField, memberValueStr.c_str());
						if (!parsedMemberValueBoxed) {
							std::cerr << std::format("Failed to deserialize field {}::{} on BehaviorComponent {}. Invalid data.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString()) << std::endl;
						}
						else {
							mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
						}
					}
					else if (mono_class_is_enum(fieldClass)) {
						auto const memberValueStr = it->second.as<std::string>();
						auto const parsedMemberValueBoxed = leopph::gManagedRuntime.ParseEnumValue(fieldRefType, memberValueStr.c_str());
						if (!parsedMemberValueBoxed) {
							std::cerr << std::format("Failed to deserialize field {}::{} on BehaviorComponent {}. Invalid data.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString()) << std::endl;
						}
						else {
							mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
						}
					}
					else if (mono_class_is_valuetype(fieldClass)) {
						auto const fieldValueBoxed = mono_field_get_value_object(leopph::gManagedRuntime.GetManagedDomain(), field, obj);
						parseAndSetMembers(fieldValueBoxed, it->second);
						mono_field_set_value(obj, field, mono_object_unbox(fieldValueBoxed));
					}
					else {
						auto const guidStr{ it->second.as<std::string>() };
						auto const targetObj{ Object::FindObjectByGuid(Guid::Parse(guidStr)) };
						if (!targetObj) {
							std::cerr << std::format("Failed to deserialize field {}::{} on BehaviorComponent {}. Guid {} does not belong to any object.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString(), guidStr) << std::endl;
						}
						else {
							auto const targetObjCast{ dynamic_cast<ManagedAccessObject*>(targetObj) };
							if (!targetObjCast) {
								std::cerr << std::format("Failed to deserialize field {}::{} on BehaviorComponent {}. Object {} is not a LeopphEngine type. Currently only LeopphEngine types are supported for by-reference serialization.", mono_type_get_name(mono_class_get_type(objClass)), memberName, GetGuid().ToString(), guidStr) << std::endl;
							}
							else {
								mono_field_set_value(obj, field, reinterpret_cast<void*>(targetObjCast->GetManagedObject()));
							}
						}
					}
				}
				else {
					std::cerr << std::format("Ignoring member \"{}\" found in file while deserializing BehaviorComponent {}. Member is not found in the assigned type.", memberName, GetGuid().ToString()) << std::endl;
				}
			}
		};

		parseAndSetMembers(managedComponent, node);
	}
}
