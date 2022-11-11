#include "SceneElement.hpp"

#include "Entity.hpp"
#include "Components.hpp"
#include "Math.hpp"
#include "ManagedRuntime.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <format>
#include <functional>
#include <iostream>
#include <variant>

namespace YAML {
	template<typename T, std::size_t N>
	struct convert<leopph::Vector<T, N>> {
		static auto encode(leopph::Vector<T, N> const& v) -> Node {
			Node node;
			node.SetStyle(YAML::EmitterStyle::Flow);
			for (std::size_t i = 0; i < N; i++) {
				node.push_back(v[i]);
			}
			return node;
		}

		static auto decode(Node const& node, leopph::Vector<T, N>& v) -> bool {
			if (!node.IsSequence() || node.size() != N) {
				return false;
			}
			for (std::size_t i = 0; i < N; i++) {
				v[i] = node[i].as<T>();
			}
			return true;
		}
	};

	template<>
	struct convert<leopph::Quaternion> {
		static auto encode(leopph::Quaternion const& q) -> Node {
			Node node;
			node.SetStyle(YAML::EmitterStyle::Flow);
			node.push_back(q.w);
			node.push_back(q.x);
			node.push_back(q.y);
			node.push_back(q.z);
			return node;
		}

		static auto decode(Node const& node, leopph::Quaternion& q) -> bool {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}
			q.w = node[0].as<leopph::f32>();
			q.x = node[1].as<leopph::f32>();
			q.y = node[2].as<leopph::f32>();
			q.z = node[3].as<leopph::f32>();
			return true;
		}
	};
}


namespace leopph {
	auto Entity::GetSerializationType() const -> Type {
		return Type::Entity;
	}


	auto Transform::GetSerializationType() const -> Type {
		return Type::Transform;
	}


	auto Camera::GetSerializationType() const -> SceneElement::Type {
		return SceneElement::Type::Camera;
	}


	auto Behavior::GetSerializationType() const -> Type {
		return Type::Behavior;
	}


	auto CubeModel::GetSerializationType() const -> Type {
		return Type::CubeModel;
	}


	auto Entity::Serialize(YAML::Node& node) const -> void {
		node["name"] = name;
		for (auto const& component : mComponents) {
			node["components"].push_back(component->GetGuid().ToString());
		}
	}


	auto Entity::Deserialize(YAML::Node const& node) -> void {
		name = node["name"].as<std::string>();
	}


	auto Entity::DeserializeResolveReferences(YAML::Node const& node) -> void {
		for (auto it{ node["components"].begin() }; it != node["components"].end(); ++it) {
			mComponents.emplace_back(static_cast<Component*>(GetObjectWithGuid(Guid::Parse(it->as<std::string>()))));
		}
	}


	auto Component::Serialize(YAML::Node& node) const -> void {
		node["entity"] = entity->guid.ToString();
	}


	auto Component::DeserializeResolveReferences(YAML::Node const& node) -> void {
		entity = static_cast<Entity*>(GetObjectWithGuid(Guid::Parse(node["entity"].as<std::string>())));
	}


	auto Transform::Serialize(YAML::Node& node) const -> void {
		Component::Serialize(node);
		node["position"] = mLocalPosition;
		node["rotation"] = mLocalRotation;
		node["scale"] = mLocalScale;
		if (mParent) {
			node["parent"] = mParent->GetGuid().ToString();
		}
		for (auto const* const child : mChildren) {
			node["children"].push_back(child->GetGuid().ToString());
		}
	}


	auto Transform::Deserialize(YAML::Node const& node) -> void {
		SetLocalPosition(node["position"].as<Vector3>());
		SetLocalRotation(node["rotation"].as<Quaternion>());
		SetLocalScale(node["scale"].as<Vector3>());
	}


	auto Transform::DeserializeResolveReferences(YAML::Node const& node) -> void {
		Component::DeserializeResolveReferences(node);
		if (node["parent"]) {
			SetParent(static_cast<Transform*>(GetObjectWithGuid(Guid::Parse(node["parent"].as<std::string>()))));
		}
		if (node["children"]) {
			for (auto it{ node["children"].begin() }; it != node["children"].end(); ++it) {
				static_cast<Transform*>(GetObjectWithGuid(Guid::Parse(it->as<std::string>())))->SetParent(this);
			}
		}
	}


	auto Camera::Serialize(YAML::Node& node) const -> void {
		Component::Serialize(node);
		node["type"] = static_cast<int>(mType);
		node["fov"] = mPerspFovHorizDeg;
		node["size"] = mOrthoSizeHoriz;
		node["near"] = mNear;
		node["far"] = mFar;
	}


	auto Camera::Deserialize(YAML::Node const& node) -> void {
		mType = static_cast<Type>(node["type"].as<int>());
		mPerspFovHorizDeg = node["fov"].as<leopph::f32>();
		mOrthoSizeHoriz = node["size"].as<leopph::f32>();
		mNear = node["near"].as<leopph::f32>();
		mFar = node["far"].as<leopph::f32>();
	}


	auto Behavior::Serialize(YAML::Node& node) const -> void {
		Component::Serialize(node);
		auto const componentClass = mono_object_get_class(GetManagedObject());
		node["classNameSpace"] = mono_class_get_namespace(componentClass);
		node["className"] = mono_class_get_name(componentClass);

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

		serializeObject(GetManagedObject(), node);
	}


	auto Behavior::Deserialize(YAML::Node const& node) -> void {
		auto const classNs = node["classNameSpace"].as<std::string>();
		auto const className = node["className"].as<std::string>();
		auto const componentClass = mono_class_from_name(leopph::GetManagedImage(), classNs.c_str(), className.c_str());
		Init(componentClass);
		auto const managedComponent = GetManagedObject();

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

		parseAndSetMembers(managedComponent, node["data"]);
	}


	auto Behavior::DeserializeResolveReferences(YAML::Node const& node) -> void {
		Component::DeserializeResolveReferences(node);
	}


	auto CubeModel::Serialize([[maybe_unused]] YAML::Node& node) const -> void {
		Component::Serialize(node);
	}


	auto CubeModel::Deserialize([[maybe_unused]] YAML::Node const& node) -> void {

	}
}