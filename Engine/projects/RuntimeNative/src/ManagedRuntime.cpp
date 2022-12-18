#include "ManagedRuntime.hpp"

#include "Entity.hpp"
#include "Time.hpp"
#include "Platform.hpp"
#include "ManagedAccessObject.hpp"
#include "Components.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <cstdio>
#include <cassert>
#include <filesystem>
#include <format>
#include <string>
#include <vector>


namespace leopph {
	namespace {
		MonoDomain* gDomain;
		MonoImage* gImage;

		std::vector<MonoClass*> gComponentClasses;

		MonoMethod* gFieldSerializeTestMethod;
		MonoMethod* gPropertySerializeTestMethod;
		MonoMethod* gGetEnumValuesMethod;
		MonoMethod* gPrimitiveTestMethod;
		MonoMethod* gParsePropertyValueMethod;
		MonoMethod* gParseFieldValueMethod;
		MonoMethod* gEnumToUnderlyingValueMethod;
		MonoMethod* gParseEnumValueMethod;
	}


	bool initialize_managed_runtime() {
		auto const printMonoLocationError = []() {
			MessageBoxW(nullptr, L"Failed to locate Mono installation.", L"Error", MB_ICONERROR);
		};

		HKEY monoRegKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Mono)", 0, KEY_READ, &monoRegKey) != ERROR_SUCCESS) {
			printMonoLocationError();
			return false;
		}

		auto const fwAssemblyDirKey = L"FrameworkAssemblyDirectory";
		DWORD bufSz{};
		if (RegGetValueW(monoRegKey, NULL, fwAssemblyDirKey, RRF_RT_REG_SZ, NULL, nullptr, &bufSz) != ERROR_SUCCESS) {
			printMonoLocationError();
			return false;
		}

		std::wstring buf(static_cast<std::size_t>(bufSz), L'\0');
		if (RegGetValueW(monoRegKey, NULL, fwAssemblyDirKey, RRF_RT_REG_SZ, NULL, buf.data(), &bufSz) != ERROR_SUCCESS) {
			printMonoLocationError();
			return false;
		}

		auto const fwAssemblyDir{ platform::WideToUtf8(buf) };

		auto const configDirKey = L"MonoConfigDir";
		if (RegGetValueW(monoRegKey, NULL, configDirKey, RRF_RT_REG_SZ, NULL, nullptr, &bufSz) != ERROR_SUCCESS) {
			printMonoLocationError();
			return false;
		}

		buf.resize(bufSz);

		if (RegGetValueW(monoRegKey, NULL, configDirKey, RRF_RT_REG_SZ, NULL, buf.data(), &bufSz) != ERROR_SUCCESS) {
			printMonoLocationError();
			return false;
		}

		auto const configDir{ platform::WideToUtf8(buf) };

		mono_set_dirs(fwAssemblyDir.c_str(), configDir.c_str());

		gDomain = mono_jit_init("leopph");

		if (!gDomain) {
			MessageBoxW(nullptr, L"Failed to create managed domain.", L"Error", MB_ICONERROR);
			return false;
		}

		mono_add_internal_call("leopph.Input::GetKeyDown", reinterpret_cast<void*>(&platform::GetKeyDown));
		mono_add_internal_call("leopph.Input::GetKey", reinterpret_cast<void*>(&platform::GetKey));
		mono_add_internal_call("leopph.Input::GetKeyUp", reinterpret_cast<void*>(&platform::GetKeyUp));
		mono_add_internal_call("leopph.Input::get_MousePosition", reinterpret_cast<void*>(&platform::GetMousePosition));
		mono_add_internal_call("leopph.Input::get_MouseDelta", reinterpret_cast<void*>(&platform::GetMouseDelta));
		mono_add_internal_call("leopph.Input::get_IsCursorConfined", reinterpret_cast<void*>(&platform::is_cursor_confined));
		mono_add_internal_call("leopph.Input::set_IsCursorConfined", reinterpret_cast<void*>(&platform::confine_cursor));
		mono_add_internal_call("leopph.Input::get_IsCursorHidden", reinterpret_cast<void*>(&platform::is_cursor_hidden));
		mono_add_internal_call("leopph.Input::set_IsCursorHidden", reinterpret_cast<void*>(&platform::hide_cursor));

		mono_add_internal_call("leopph.Time::get_FullTime", reinterpret_cast<void*>(&managedbindings::get_full_time));
		mono_add_internal_call("leopph.Time::get_FrameTime", reinterpret_cast<void*>(&managedbindings::get_frame_time));

		mono_add_internal_call("leopph.Window::get_CurrentResolution", reinterpret_cast<void*>(&platform::get_window_current_client_area_size));
		mono_add_internal_call("leopph.Window::get_WindowedResolution", reinterpret_cast<void*>(&platform::get_window_windowed_client_area_size));
		mono_add_internal_call("leopph.Window::set_WindowedResolution", reinterpret_cast<void*>(&platform::set_window_windowed_client_area_size));
		mono_add_internal_call("leopph.Window::get_IsBorderless", reinterpret_cast<void*>(&platform::managedbindings::IsWindowBorderLess));
		mono_add_internal_call("leopph.Window::set_IsBorderless", reinterpret_cast<void*>(&platform::managedbindings::SetWindowBorderless));
		mono_add_internal_call("leopph.Window::get_IsMinimizingOnBorderlessFocusLoss", reinterpret_cast<void*>(&platform::managedbindings::IsWindowMinimizingOnBorderlessFocusLoss));
		mono_add_internal_call("leopph.Window::set_IsMinimizingOnBorderlessFocusLoss", reinterpret_cast<void*>(&platform::managedbindings::SetWindowMinimizeOnBorderlessFocusLoss));
		mono_add_internal_call("leopph.Window::InternalSetShouldClose", reinterpret_cast<void*>(&platform::managedbindings::SetWindowShouldClose));

		mono_add_internal_call("leopph.NativeWrapper.get_Guid", reinterpret_cast<void*>(&managedbindings::GetManagedAccessObjectGuid));

		mono_add_internal_call("leopph.Entity::NewNativeEntity", reinterpret_cast<void*>(&managedbindings::CreateNativeEntity));
		mono_add_internal_call("leopph.Entity::get_Name", reinterpret_cast<void*>(&managedbindings::GetEntityName));
		mono_add_internal_call("leopph.Entity::set_Name", reinterpret_cast<void*>(&managedbindings::SetEntityName));
		mono_add_internal_call("leopph.Entity::get_Transform", reinterpret_cast<void*>(&managedbindings::GetEntityTransform));
		mono_add_internal_call("leopph.Entity::InternalCreateComponent", reinterpret_cast<void*>(&managedbindings::EntityCreateComponent));

		mono_add_internal_call("leopph.Component::get_Entity", reinterpret_cast<void*>(&managedbindings::GetComponentEntity));
		mono_add_internal_call("leopph.Component::get_Transform", reinterpret_cast<void*>(&managedbindings::GetComponentEntityTransform));

		mono_add_internal_call("leopph.Transform::get_WorldPosition", reinterpret_cast<void*>(&managedbindings::GetTransformWorldPosition));
		mono_add_internal_call("leopph.Transform::set_WorldPosition", reinterpret_cast<void*>(&managedbindings::SetTransformWorldPosition));
		mono_add_internal_call("leopph.Transform::get_LocalPosition", reinterpret_cast<void*>(&managedbindings::GetTransformLocalPosition));
		mono_add_internal_call("leopph.Transform::set_LocalPosition", reinterpret_cast<void*>(&managedbindings::SetTransformLocalPosition));
		mono_add_internal_call("leopph.Transform::get_WorldRotation", reinterpret_cast<void*>(&managedbindings::GetTransformWorldRotation));
		mono_add_internal_call("leopph.Transform::set_WorldRotation", reinterpret_cast<void*>(&managedbindings::SetTransformWorldRotation));
		mono_add_internal_call("leopph.Transform::get_LocalRotation", reinterpret_cast<void*>(&managedbindings::GetTransformLocalRotation));
		mono_add_internal_call("leopph.Transform::set_LocalRotation", reinterpret_cast<void*>(&managedbindings::SetTransformLocalRotation));
		mono_add_internal_call("leopph.Transform::get_WorldScale", reinterpret_cast<void*>(&managedbindings::GetTransformWorldScale));
		mono_add_internal_call("leopph.Transform::set_WorldScale", reinterpret_cast<void*>(&managedbindings::SetTransformWorldScale));
		mono_add_internal_call("leopph.Transform::get_LocalScale", reinterpret_cast<void*>(&managedbindings::GetTransformLocalScale));
		mono_add_internal_call("leopph.Transform::set_LocalScale", reinterpret_cast<void*>(&managedbindings::SetTransformLocalScale));
		mono_add_internal_call("leopph.Transform::get_Right", reinterpret_cast<void*>(&managedbindings::GetTransformRightAxis));
		mono_add_internal_call("leopph.Transform::get_Up", reinterpret_cast<void*>(&managedbindings::GetTransformUpAxis));
		mono_add_internal_call("leopph.Transform::get_Forward", reinterpret_cast<void*>(&managedbindings::GetTransformForwardAxis));
		mono_add_internal_call("leopph.Transform::get_Parent", reinterpret_cast<void*>(&managedbindings::GetTransformParent));
		mono_add_internal_call("leopph.Transform::set_Parent", reinterpret_cast<void*>(&managedbindings::SetTransformParent));
		mono_add_internal_call("leopph.Transform::get_ModelMatrix", reinterpret_cast<void*>(&managedbindings::GetTransformModelMatrix));
		mono_add_internal_call("leopph.Transform::get_NormalMatrix", reinterpret_cast<void*>(&managedbindings::GetTransformNormalMatrix));
		mono_add_internal_call("leopph.Transform::Translate(leopph.Vector3,leopph.Space)", reinterpret_cast<void*>(&managedbindings::TranslateTransformVector));
		mono_add_internal_call("leopph.Transform::Translate(single,single,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::TranslateTransform));
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Quaternion,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RotateTransform));
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Vector3,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RotateTransformAngleAxis));
		mono_add_internal_call("leopph.Transform::Rescale(leopph.Vector3,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RescaleTransformVector));
		mono_add_internal_call("leopph.Transform::Rescale(single,single,single,leopph.Space)", reinterpret_cast<void*>(&managedbindings::RescaleTransform));

		mono_add_internal_call("leopph.Camera::get_Type", reinterpret_cast<void*>(&managedbindings::GetCameraType));
		mono_add_internal_call("leopph.Camera::set_Type", reinterpret_cast<void*>(&managedbindings::SetCameraType));
		mono_add_internal_call("leopph.Camera::get_PerspectiveFieldOfView", reinterpret_cast<void*>(&managedbindings::GetCameraPerspectiveFov));
		mono_add_internal_call("leopph.Camera::set_PerspectiveFieldOfView", reinterpret_cast<void*>(&managedbindings::SetCameraPerspectiveFov));
		mono_add_internal_call("leopph.Camera::get_OrthographicSize", reinterpret_cast<void*>(&managedbindings::GetCameraOrthographicSize));
		mono_add_internal_call("leopph.Camera::set_OrthographicSize", reinterpret_cast<void*>(&managedbindings::SetCameraOrthographicSize));
		mono_add_internal_call("leopph.Camera::get_NearClipPlane", reinterpret_cast<void*>(&managedbindings::GetCameraNearClipPlane));
		mono_add_internal_call("leopph.Camera::set_NearClipPlane", reinterpret_cast<void*>(&managedbindings::SetCameraNearClipPlane));
		mono_add_internal_call("leopph.Camera::get_FarClipPlane", reinterpret_cast<void*>(&managedbindings::GetCameraFarClipPlane));
		mono_add_internal_call("leopph.Camera::set_FarClipPlane", reinterpret_cast<void*>(&managedbindings::SetCameraFarClipPlane));

		mono_add_internal_call("leopph.Light::get_Color", reinterpret_cast<void*>(&managedbindings::GetLightColor));
		mono_add_internal_call("leopph.Light::set_Color", reinterpret_cast<void*>(&managedbindings::SetLightColor));
		mono_add_internal_call("leopph.Light::get_Intensity", reinterpret_cast<void*>(&managedbindings::GetLightIntensity));
		mono_add_internal_call("leopph.Light::set_Intensity", reinterpret_cast<void*>(&managedbindings::SetLightIntensity));

		auto const managedLibPath{ std::filesystem::path{ platform::GetExecutablePath() }.remove_filename() /= "LeopphRuntimeManaged.dll" };

		auto const assembly{ mono_domain_assembly_open(gDomain, platform::WideToUtf8(managedLibPath.c_str()).c_str()) };

		if (!assembly) {
			MessageBoxW(nullptr, std::format(L"Failed to open managed assembly at {}.", managedLibPath.c_str()).c_str(), L"Error", MB_ICONERROR);
			return false;
		}

		gImage = mono_assembly_get_image(assembly);

		if (!gImage) {
			MessageBoxW(nullptr, L"Failed to image from managed assembly.", L"Error", MB_ICONERROR);
			return false;
		}

		auto const helperClass = mono_class_from_name(gImage, "leopph", "NativeHelpers");
		assert(helperClass);
		gFieldSerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeField", 1);
		assert(gFieldSerializeTestMethod);
		gPropertySerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeProperty", 1);
		assert(gPropertySerializeTestMethod);
		gGetEnumValuesMethod = mono_class_get_method_from_name(helperClass, "GetEnumValues", 1);
		assert(gGetEnumValuesMethod);
		gPrimitiveTestMethod = mono_class_get_method_from_name(helperClass, "IsPrimitive", 1);
		assert(gPrimitiveTestMethod);
		gParsePropertyValueMethod = mono_class_get_method_from_name(helperClass, "ParsePropertyValue", 2);
		assert(gParsePropertyValueMethod);
		gParseFieldValueMethod = mono_class_get_method_from_name(helperClass, "ParseFieldValue", 2);
		assert(gParseFieldValueMethod);
		gEnumToUnderlyingValueMethod = mono_class_get_method_from_name(helperClass, "EnumToUnderlyingType", 2);
		assert(gEnumToUnderlyingValueMethod);
		gParseEnumValueMethod = mono_class_get_method_from_name(helperClass, "ParseEnumValue", 2);
		assert(gParseEnumValueMethod);

		auto const componentClass = mono_class_from_name(gImage, "leopph", "Component");
		auto const behaviorClass = mono_class_from_name(gImage, "leopph", "Behavior");

		auto const table = mono_image_get_table_info(gImage, MONO_TABLE_TYPEDEF);
		auto const numRows = mono_table_info_get_rows(table);

		for (int i = 0; i < numRows; i++) {
			u32 cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table, i, cols, MONO_TYPEDEF_SIZE);
			auto const mName = mono_metadata_string_heap(gImage, cols[MONO_TYPEDEF_NAME]);
			auto const ns = mono_metadata_string_heap(gImage, cols[MONO_TYPEDEF_NAMESPACE]);
			auto const klass = mono_class_from_name(gImage, ns, mName);

			if (mono_class_is_subclass_of(klass, componentClass, false) && klass != componentClass && klass != behaviorClass) {
				gComponentClasses.emplace_back(klass);
			}
		}

		return true;
	}


	void cleanup_managed_runtime() {
		mono_jit_cleanup(gDomain);
	}


	MonoImage* GetManagedImage() {
		return gImage;
	}


	MonoDomain* GetManagedDomain() {
		return gDomain;
	}


	bool ShouldSerialize(MonoReflectionField* field) {
		auto const res = mono_runtime_invoke(gFieldSerializeTestMethod, nullptr, reinterpret_cast<void**>(&field), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	bool ShouldSerialize(MonoReflectionProperty* prop) {
		auto const res = mono_runtime_invoke(gPropertySerializeTestMethod, nullptr, reinterpret_cast<void**>(&prop), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	MonoArray* GetEnumValues(MonoReflectionType* enumType) {
		return reinterpret_cast<MonoArray*>(mono_runtime_invoke(gGetEnumValuesMethod, nullptr, reinterpret_cast<void**>(&enumType), nullptr));
	}


	std::span<MonoClass* const> GetComponentClasses() {
		return gComponentClasses;
	}


	bool IsTypePrimitive(MonoReflectionType* refType) {
		return *reinterpret_cast<int*>(mono_object_unbox(mono_runtime_invoke(gPrimitiveTestMethod, nullptr, reinterpret_cast<void**>(&refType), nullptr)));
	}


	MonoObject* ParseValue(MonoReflectionField* field, std::string_view const str) {
		auto managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(field), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(gParseFieldValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}


	MonoObject* ParseValue(MonoReflectionProperty* property, std::string_view const str) {
		auto managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(property), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(gParsePropertyValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}


	MonoObject* EnumToUnderlyingType(MonoReflectionType* enumType, MonoObject* enumValue) {
		void* params[]{ reinterpret_cast<void*>(enumType), reinterpret_cast<void*>(enumValue) };
		auto const underlyingValue = mono_runtime_invoke(gEnumToUnderlyingValueMethod, nullptr, params, nullptr);
		return underlyingValue;
	}


	MonoObject* ParseEnumValue(MonoReflectionType* enumType, std::string_view const str) {
		auto managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(enumType), reinterpret_cast<void*>(managedStr) };
		auto const parsedValue = mono_runtime_invoke(gParseEnumValueMethod, nullptr, params, nullptr);
		return parsedValue;
	}
}