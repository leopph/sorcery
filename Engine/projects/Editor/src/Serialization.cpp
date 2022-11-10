#include "Serialization.hpp"

#include <Entity.hpp>
#include <Components.hpp>
#include <ManagedRuntime.hpp>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <format>
#include <functional>
#include <iostream>
#include <variant>

namespace leopph::editor {
	auto SerializeScene() -> YAML::Node {
		YAML::Node scene;
		static std::vector<leopph::Entity*> entities;
		for (auto const& entity : leopph::GetEntities(entities)) {
			YAML::Node entityNode;
			entityNode["name"] = entity->name;

			static std::vector<leopph::Component*> components;

			for (auto const& component : entity->GetComponents(components)) {
				YAML::Node componentNode;
				auto const componentClass = mono_object_get_class(component->GetManagedObject());
				componentNode["classNameSpace"] = mono_class_get_namespace(componentClass);
				componentNode["className"] = mono_class_get_name(componentClass);

				std::function<void(MonoObject*, YAML::Node&)> serializeObject;

				serializeObject = [&serializeObject](MonoObject* const obj, YAML::Node& node) -> void {
					auto const objClass = mono_object_get_class(obj);

					void* iter{ nullptr };

					while (auto const field = mono_class_get_fields(objClass, &iter)) {
						if (leopph::ShouldSerialize(mono_field_get_object(leopph::GetManagedDomain(), objClass, field))) {
							auto const fieldType = mono_field_get_type(field);
							auto const fieldRefType = mono_type_get_object(leopph::GetManagedDomain(), fieldType);
							auto const fieldClass = mono_type_get_class(fieldType);
							auto const fieldName = mono_field_get_name(field);
							auto const fieldValueBoxed = mono_field_get_value_object(leopph::GetManagedDomain(), field, obj);

							if (leopph::IsTypePrimitive(fieldRefType)) {
								node[fieldName] = mono_string_to_utf8(mono_object_to_string(fieldValueBoxed, nullptr));
							}
							else if (mono_class_is_enum(fieldClass)) {
								node[fieldName] = mono_string_to_utf8(mono_object_to_string(leopph::EnumToUnderlyingType(fieldRefType, fieldValueBoxed), nullptr));
							}
							else if (mono_class_is_valuetype(fieldClass)) {
								auto dataNode = node[fieldName];
								serializeObject(fieldValueBoxed, dataNode);
							}
							else {
								std::cerr << "Serialization of reference type fields is not yet supported." << std::endl;
							}
						}
					}

					iter = nullptr;

					while (auto const prop = mono_class_get_properties(objClass, &iter)) {
						if (leopph::ShouldSerialize(mono_property_get_object(leopph::GetManagedDomain(), objClass, prop))) {
							auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
							auto const propRefType = mono_type_get_object(leopph::GetManagedDomain(), propType);
							auto const propClass = mono_type_get_class(propType);
							auto const propName = mono_property_get_name(prop);
							auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
							auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);

							if (leopph::IsTypePrimitive(propRefType)) {
								node[propName] = mono_string_to_utf8(mono_object_to_string(propValueBoxed, nullptr));
							}
							else if (mono_class_is_enum(propClass)) {
								node[propName] = mono_string_to_utf8(mono_object_to_string(leopph::EnumToUnderlyingType(propRefType, propValueBoxed), nullptr));
							}
							else if (mono_class_is_valuetype(propClass)) {
								auto dataNode = node[propName];
								serializeObject(propValueBoxed, dataNode);
							}
							else {
								std::cerr << "Serialization of reference type properties is not yet supported." << std::endl;
							}
						}
					}
				};

				auto dataNode = componentNode["data"];
				serializeObject(component->GetManagedObject(), dataNode);
				entityNode["components"].push_back(componentNode);
			}

			scene.push_back(entityNode);
		}

		return scene;
	}


	auto DeserializeScene(YAML::Node const& scene) -> void {
		for (auto const& entityNode : scene) {
			auto const entity = leopph::Entity::Create();
			entity->name = entityNode["name"].as<std::string>();
			entity->CreateManagedObject("leopph", "Entity");

			for (auto const& componentNode : entityNode["components"]) {
				auto const classNs = componentNode["classNameSpace"].as<std::string>();
				auto const className = componentNode["className"].as<std::string>();
				auto const componentClass = mono_class_from_name(leopph::GetManagedImage(), classNs.c_str(), className.c_str());
				auto const component = entity->CreateComponent(componentClass);
				auto const managedComponent = component->GetManagedObject();

				std::function<void(MonoObject*, YAML::Node const&)> parseAndSetMembers;
				std::function<MonoObject* (YAML::Node const&, MonoObject*, std::variant<MonoProperty*, MonoClassField*>)> setMember;

				parseAndSetMembers = [&parseAndSetMembers](MonoObject* const obj, YAML::Node const& dataNode) -> void {
					auto const objClass = mono_object_get_class(obj);

					for (auto it = dataNode.begin(); it != dataNode.end(); ++it) {
						auto const memberName = it->first.as<std::string>();

						if (auto const prop = mono_class_get_property_from_name(objClass, memberName.data())) {
							auto const propType = mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop)));
							auto const propRefType = mono_type_get_object(leopph::GetManagedDomain(), propType);
							auto const propClass = mono_type_get_class(propType);
							auto const objPossiblyUnboxed = mono_class_is_valuetype(objClass) ? mono_object_unbox(obj) : obj;
							auto const propValueBoxed = mono_property_get_value(prop, objPossiblyUnboxed, nullptr, nullptr);
							auto const refProp = mono_property_get_object(leopph::GetManagedDomain(), objClass, prop);

							if (leopph::IsTypePrimitive(propRefType)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseValue(refProp, memberValueStr.c_str());
								auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
							}
							else if (mono_class_is_enum(propClass)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseEnumValue(propRefType, memberValueStr.c_str());
								auto parsedMemberValueUnboxed = mono_object_unbox(parsedMemberValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &parsedMemberValueUnboxed, nullptr);
							}
							else if (mono_class_is_valuetype(propClass)) {
								parseAndSetMembers(propValueBoxed, it->second);
								auto propValueUnboxed = mono_object_unbox(propValueBoxed);
								mono_property_set_value(prop, objPossiblyUnboxed, &propValueUnboxed, nullptr);
							}
							else {
								std::cerr << "Deserialization of reference type properties is not yet supported." << std::endl;
							}
						}
						else if (auto const field = mono_class_get_field_from_name(objClass, memberName.data())) {
							auto const fieldType = mono_field_get_type(field);
							auto const fieldRefType = mono_type_get_object(leopph::GetManagedDomain(), fieldType);
							auto const fieldClass = mono_type_get_class(fieldType);
							auto const refField = mono_field_get_object(leopph::GetManagedDomain(), objClass, field);

							if (leopph::IsTypePrimitive(fieldRefType)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseValue(refField, memberValueStr.c_str());
								mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
							}
							else if (mono_class_is_enum(fieldClass)) {
								auto const memberValueStr = it->second.as<std::string>();
								auto const parsedMemberValueBoxed = leopph::ParseEnumValue(fieldRefType, memberValueStr.c_str());
								mono_field_set_value(obj, field, mono_object_unbox(parsedMemberValueBoxed));
							}
							else if (mono_class_is_valuetype(fieldClass)) {
								auto const fieldValueBoxed = mono_field_get_value_object(leopph::GetManagedDomain(), field, obj);
								parseAndSetMembers(fieldValueBoxed, it->second);
								mono_field_set_value(obj, field, mono_object_unbox(fieldValueBoxed));
							}
							else {
								std::cerr << "Deserialization of reference type fields is not yet supported." << std::endl;
							}
						}
						else {
							std::cerr << std::format("Member \"{}\" in file has no corresponding member in class.", memberName) << std::endl;
						}
					}
				};

				parseAndSetMembers(managedComponent, componentNode["data"]);
			}
		}
	}
}