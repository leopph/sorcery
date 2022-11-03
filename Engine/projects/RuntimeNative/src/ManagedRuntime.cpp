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
#include <string>
#include <vector>


namespace leopph
{
	namespace
	{
		MonoDomain* gDomain;
		MonoImage* gImage;
		MonoMethod* gFieldSerializeTestMethod;
		MonoMethod* gPropertySerializeTestMethod;
		MonoMethod* gEnumEnumeratorMethod;
		std::vector<MonoClass*> gComponentClasses;
		MonoMethod* gPrimitiveTestMethod;
		MonoMethod* gParseSetPropertyMethod;
	}


	bool initialize_managed_runtime()
	{
		auto const printMonoLocationError = []()
		{
			MessageBoxW(nullptr, L"Failed to locate Mono installation.", L"Error", MB_ICONERROR);
		};

		HKEY monoRegKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Mono)", 0, KEY_READ, &monoRegKey) != ERROR_SUCCESS)
		{
			printMonoLocationError();
			return false;
		}

		auto const fwAssemblyDirKey = L"FrameworkAssemblyDirectory";
		DWORD bufSz{};
		if (RegGetValueW(monoRegKey, NULL, fwAssemblyDirKey, RRF_RT_REG_SZ, NULL, nullptr, &bufSz) != ERROR_SUCCESS)
		{
			printMonoLocationError();
			return false;
		}

		std::wstring buf(static_cast<std::size_t>(bufSz), L'\0');
		if (RegGetValueW(monoRegKey, NULL, fwAssemblyDirKey, RRF_RT_REG_SZ, NULL, buf.data(), &bufSz) != ERROR_SUCCESS)
		{
			printMonoLocationError();
			return false;
		}

		std::string fwAssemblyDir(static_cast<std::size_t>(WideCharToMultiByte(CP_UTF8, 0, buf.data(), static_cast<int>(buf.size()), nullptr, 0, nullptr, nullptr)), '\0');
		WideCharToMultiByte(CP_UTF8, 0, buf.data(), static_cast<int>(buf.size()), fwAssemblyDir.data(), static_cast<int>(fwAssemblyDir.size()), nullptr, nullptr);

		auto const configDirKey = L"MonoConfigDir";
		if (RegGetValueW(monoRegKey, NULL, configDirKey, RRF_RT_REG_SZ, NULL, nullptr, &bufSz) != ERROR_SUCCESS)
		{
			printMonoLocationError();
			return false;
		}

		buf.resize(bufSz);

		if (RegGetValueW(monoRegKey, NULL, configDirKey, RRF_RT_REG_SZ, NULL, buf.data(), &bufSz) != ERROR_SUCCESS)
		{
			printMonoLocationError();
			return false;
		}

		std::string configDir(static_cast<std::size_t>(WideCharToMultiByte(CP_UTF8, 0, buf.data(), static_cast<int>(buf.size()), nullptr, 0, nullptr, nullptr)), '\0');
		WideCharToMultiByte(CP_UTF8, 0, buf.data(), static_cast<int>(buf.size()), configDir.data(), static_cast<int>(configDir.size()), nullptr, nullptr);

		mono_set_dirs(fwAssemblyDir.c_str(), configDir.c_str());

		gDomain = mono_jit_init("leopph");
		assert(gDomain);

		mono_add_internal_call("leopph.Input::GetKeyDown", platform::GetKeyDown);
		mono_add_internal_call("leopph.Input::GetKey", platform::GetKey);
		mono_add_internal_call("leopph.Input::GetKeyUp", platform::GetKeyUp);
		mono_add_internal_call("leopph.Input::get_MousePosition", platform::GetMousePosition);
		mono_add_internal_call("leopph.Input::get_MouseDelta", platform::GetMouseDelta);
		mono_add_internal_call("leopph.Input::get_IsCursorConfined", platform::is_cursor_confined);
		mono_add_internal_call("leopph.Input::set_IsCursorConfined", platform::confine_cursor);
		mono_add_internal_call("leopph.Input::get_IsCursorHidden", platform::is_cursor_hidden);
		mono_add_internal_call("leopph.Input::set_IsCursorHidden", platform::hide_cursor);

		mono_add_internal_call("leopph.Time::get_FullTime", managedbindings::get_full_time);
		mono_add_internal_call("leopph.Time::get_FrameTime", managedbindings::get_frame_time);

		mono_add_internal_call("leopph.Window::get_CurrentResolution", platform::get_window_current_client_area_size);
		mono_add_internal_call("leopph.Window::get_WindowedResolution", platform::get_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::set_WindowedResolution", platform::set_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::get_IsBorderless", platform::managedbindings::IsWindowBorderLess);
		mono_add_internal_call("leopph.Window::set_IsBorderless", platform::managedbindings::SetWindowBorderless);
		mono_add_internal_call("leopph.Window::get_IsMinimizingOnBorderlessFocusLoss", platform::managedbindings::IsWindowMinimizingOnBorderlessFocusLoss);
		mono_add_internal_call("leopph.Window::set_IsMinimizeOnBorderlessFocusLoss", platform::managedbindings::SetWindowMinimizeOnBorderlessFocusLoss);
		mono_add_internal_call("leopph.Window::InternalSetShouldClose", platform::managedbindings::SetWindowShouldClose);

		mono_add_internal_call("leopph.Entity::NewNativeEntity", managedbindings::CreateNativeEntity);
		mono_add_internal_call("leopph.Entity::get_Name", managedbindings::GetEntityName);
		mono_add_internal_call("leopph.Entity::set_Name", managedbindings::SetEntityName);
		mono_add_internal_call("leopph.Entity::get_Transform", managedbindings::GetEntityTransform);
		mono_add_internal_call("leopph.Entity::InternalCreateComponent", managedbindings::EntityCreateComponent);

		mono_add_internal_call("leopph.Component::get_Entity", managedbindings::GetComponentEntity);
		mono_add_internal_call("leopph.Component::get_Transform", managedbindings::GetComponentEntityTransform);

		mono_add_internal_call("leopph.Transform::get_WorldPosition", managedbindings::GetTransformWorldPosition);
		mono_add_internal_call("leopph.Transform::set_WorldPosition", managedbindings::SetTransformWorldPosition);
		mono_add_internal_call("leopph.Transform::get_LocalPosition", managedbindings::GetTransformLocalPosition);
		mono_add_internal_call("leopph.Transform::set_LocalPosition", managedbindings::SetTransformLocalPosition);
		mono_add_internal_call("leopph.Transform::get_WorldRotation", managedbindings::GetTransformWorldRotation);
		mono_add_internal_call("leopph.Transform::set_WorldRotation", managedbindings::SetTransformWorldRotation);
		mono_add_internal_call("leopph.Transform::get_LocalRotation", managedbindings::GetTransformLocalRotation);
		mono_add_internal_call("leopph.Transform::set_LocalRotation", managedbindings::SetTransformLocalRotation);
		mono_add_internal_call("leopph.Transform::get_WorldScale", managedbindings::GetTransformWorldScale);
		mono_add_internal_call("leopph.Transform::set_WorldScale", managedbindings::SetTransformWorldScale);
		mono_add_internal_call("leopph.Transform::get_LocalScale", managedbindings::GetTransformLocalScale);
		mono_add_internal_call("leopph.Transform::set_LocalScale", managedbindings::SetTransformLocalScale);
		mono_add_internal_call("leopph.Transform::get_Right", managedbindings::GetTransformRightAxis);
		mono_add_internal_call("leopph.Transform::get_Up", managedbindings::GetTransformUpAxis);
		mono_add_internal_call("leopph.Transform::get_Forward", managedbindings::GetTransformForwardAxis);
		mono_add_internal_call("leopph.Transform::get_Parent", managedbindings::GetTransformParent);
		mono_add_internal_call("leopph.Transform::set_Parent", managedbindings::SetTransformParent);
		mono_add_internal_call("leopph.Transform::get_ModelMatrix", managedbindings::GetTransformModelMatrix);
		mono_add_internal_call("leopph.Transform::get_NormalMatrix", managedbindings::GetTransformNormalMatrix);
		mono_add_internal_call("leopph.Transform::Translate(leopph.Vector3,leopph.Space)", managedbindings::TranslateTransformVector);
		mono_add_internal_call("leopph.Transform::Translate(single,single,single,leopph.Space)", managedbindings::TranslateTransform);
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Quaternion,leopph.Space)", managedbindings::RotateTransform);
		mono_add_internal_call("leopph.Transform::Rotate(leopph.Vector3,single,leopph.Space)", managedbindings::RotateTransformAngleAxis);
		mono_add_internal_call("leopph.Transform::Rescale(leopph.Vector3,leopph.Space)", managedbindings::RescaleTransformVector);
		mono_add_internal_call("leopph.Transform::Rescale(single,single,single,leopph.Space)", managedbindings::RescaleTransform);
		
		mono_add_internal_call("leopph.Camera::get_Type", managedbindings::GetCameraType);
		mono_add_internal_call("leopph.Camera::set_Type", managedbindings::SetCameraType);
		mono_add_internal_call("leopph.Camera::get_PerspectiveFieldOfView", managedbindings::GetCameraPerspectiveFov);
		mono_add_internal_call("leopph.Camera::set_PerspectiveFieldOfView", managedbindings::SetCameraPerspectiveFov);
		mono_add_internal_call("leopph.Camera::get_OrthographicSize", managedbindings::GetCameraOrthographicSize);
		mono_add_internal_call("leopph.Camera::set_OrthographicSize", managedbindings::SetCameraOrthographicSize);
		mono_add_internal_call("leopph.Camera::get_NearClipPlane", managedbindings::GetCameraNearClipPlane);
		mono_add_internal_call("leopph.Camera::set_NearClipPlane", managedbindings::SetCameraNearClipPlane);
		mono_add_internal_call("leopph.Camera::get_FarClipPlane", managedbindings::GetCameraFarClipPlane);
		mono_add_internal_call("leopph.Camera::set_FarClipPlane", managedbindings::SetCameraFarClipPlane);

		char* exePath;
		_get_pgmptr(&exePath);
		std::filesystem::path managedLibPath{ exePath };
		managedLibPath = managedLibPath.remove_filename();
		managedLibPath /= "LeopphRuntimeManaged.dll";

		MonoAssembly* assembly = mono_domain_assembly_open(gDomain, managedLibPath.string().c_str());
		assert(assembly);

		gImage = mono_assembly_get_image(assembly);
		assert(gImage);

		auto const helperClass = mono_class_from_name(gImage, "leopph", "NativeHelpers");
		assert(helperClass);
		gFieldSerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeField", 1);
		assert(gFieldSerializeTestMethod);
		gPropertySerializeTestMethod = mono_class_get_method_from_name(helperClass, "ShouldSerializeProperty", 1);
		assert(gPropertySerializeTestMethod);
		gEnumEnumeratorMethod = mono_class_get_method_from_name(helperClass, "GetEnumValues", 1);
		assert(gEnumEnumeratorMethod);
		gPrimitiveTestMethod = mono_class_get_method_from_name(helperClass, "IsPrimitiveOrString", 1);
		assert(gPrimitiveTestMethod);
		gParseSetPropertyMethod = mono_class_get_method_from_name(helperClass, "ParseAndSetPropertyValue", 3);
		assert(gParseSetPropertyMethod);

		auto const componentClass = mono_class_from_name(gImage, "leopph", "Component");

		auto const table = mono_image_get_table_info(gImage, MONO_TABLE_TYPEDEF);
		auto const numRows = mono_table_info_get_rows(table);

		for (int i = 0; i < numRows; i++)
		{
			u32 cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(table, i, cols, MONO_TYPEDEF_SIZE);
			auto const name = mono_metadata_string_heap(gImage, cols[MONO_TYPEDEF_NAME]);
			auto const ns = mono_metadata_string_heap(gImage, cols[MONO_TYPEDEF_NAMESPACE]);
			auto const klass = mono_class_from_name(gImage, ns, name);

			if (mono_class_is_subclass_of(klass, componentClass, false))
			{
				gComponentClasses.emplace_back();
			}
		}

		MonoClass* testClass = mono_class_from_name(gImage, "", "Test");
		MonoMethod* doTestMethod = mono_class_get_method_from_name(testClass, "DoTest", 0);
		mono_runtime_invoke(doTestMethod, nullptr, nullptr, nullptr);

		return true;
	}


	void cleanup_managed_runtime()
	{
		mono_jit_cleanup(gDomain);
	}


	MonoImage* GetManagedImage()
	{
		return gImage;
	}


	MonoDomain* GetManagedDomain()
	{
		return gDomain;
	}


	bool ShouldSerialize(MonoReflectionField* field)
	{
		auto const res =  mono_runtime_invoke(gFieldSerializeTestMethod, nullptr, reinterpret_cast<void**>(&field), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	bool ShouldSerialize(MonoReflectionProperty* prop)
	{
		auto const res = mono_runtime_invoke(gPropertySerializeTestMethod, nullptr, reinterpret_cast<void**>(&prop), nullptr);

		return *static_cast<bool*>(mono_object_unbox(res));
	}


	MonoArray* GetEnumValues(MonoType* enumType)
	{
		auto typeName = mono_string_new_wrapper(mono_type_get_name(enumType));
		return reinterpret_cast<MonoArray*>(mono_runtime_invoke(gEnumEnumeratorMethod, nullptr, reinterpret_cast<void**>(&typeName), nullptr));
	}


	std::span<MonoClass* const> GetComponentClasses()
	{
		return gComponentClasses;
	}

	bool IsTypePrimitiveOrString(MonoReflectionType* refType)
	{
		return *reinterpret_cast<int*>(mono_object_unbox(mono_runtime_invoke(gPrimitiveTestMethod, nullptr, reinterpret_cast<void**>(&refType), nullptr)));
	}


	void ParseAndSetProperty(MonoObject* object, MonoReflectionProperty* property, std::string_view str)
	{
		auto managedStr = mono_string_new_wrapper(str.data());
		void* params[]{ reinterpret_cast<void*>(object), reinterpret_cast<void*>(property), reinterpret_cast<void*>(managedStr) };
		mono_runtime_invoke(gParseSetPropertyMethod, nullptr, params, nullptr);
	}
}