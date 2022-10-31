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

		mono_add_internal_call("leopph.Input::GetKeyDown", platform::managedbindings::get_key_down);
		mono_add_internal_call("leopph.Input::GetKey", platform::managedbindings::get_key);
		mono_add_internal_call("leopph.Input::GetKeyUp", platform::managedbindings::get_key_up);
		mono_add_internal_call("leopph.Input::get_MousePosition", platform::managedbindings::get_mouse_pos);
		mono_add_internal_call("leopph.Input::get_MouseDelta", platform::managedbindings::get_mouse_delta);
		mono_add_internal_call("leopph.Input::get_IsCursorConfined", platform::is_cursor_confined);
		mono_add_internal_call("leopph.Input::set_IsCursorConfined", platform::confine_cursor);
		mono_add_internal_call("leopph.Input::get_IsCursorHidden", platform::is_cursor_hidden);
		mono_add_internal_call("leopph.Input::set_IsCursorHidden", platform::hide_cursor);

		mono_add_internal_call("leopph.Time::get_FullTime", managedbindings::get_full_time);
		mono_add_internal_call("leopph.Time::get_FrameTime", managedbindings::get_frame_time);

		mono_add_internal_call("leopph.Window::InternalGetCurrentClientAreaSize", platform::get_window_current_client_area_size);
		mono_add_internal_call("leopph.Window::InternalGetWindowedClientAreaSize", platform::get_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::InternalSetWindowedClientAreaSize", platform::set_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::InternalIsBorderless", platform::is_window_borderless);
		mono_add_internal_call("leopph.Window::InternalSetBorderless", platform::set_window_borderless);
		mono_add_internal_call("leopph.Window::InternalIsMinimizingOnBorderlessFocusLoss", platform::is_window_minimizing_on_borderless_focus_loss);
		mono_add_internal_call("leopph.Window::InternalSetMinimizeOnBorderlessFocusLoss", platform::set_window_minimize_on_borderless_focus_loss);
		mono_add_internal_call("leopph.Window::InternalSetShouldClose", platform::set_should_window_close);

		mono_add_internal_call("leopph.Entity::NewNativeEntity", managedbindings::CreateNativeEntity);
		mono_add_internal_call("leopph.Entity::get_Name", managedbindings::GetEntityName);
		mono_add_internal_call("leopph.Entity::set_Name", managedbindings::SetEntityName);
		mono_add_internal_call("leopph.Entity::get_Transform", managedbindings::GetEntityTransform);
		mono_add_internal_call("leopph.Entity::InternalCreateComponent", managedbindings::CreateComponent);

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

		char* exePath;
		_get_pgmptr(&exePath);
		std::filesystem::path managedLibPath{ exePath };
		managedLibPath = managedLibPath.remove_filename();
		managedLibPath /= "LeopphRuntimeManaged.dll";

		MonoAssembly* assembly = mono_domain_assembly_open(gDomain, managedLibPath.string().c_str());
		assert(assembly);

		gImage = mono_assembly_get_image(assembly);
		assert(gImage);

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
}