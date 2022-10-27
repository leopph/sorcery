#include "Managed.hpp"

#include "Behavior.hpp"
#include "CubeModel.hpp"
#include "Entity.hpp"
#include "Time.hpp"
#include "Platform.hpp"
#include "ManagedAccessObject.hpp"
#include "Component.hpp"

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

		mono_add_internal_call("leopph.NativeWrapper::InternalDestroyMAO", destroy_mao);

		mono_add_internal_call("leopph.Entity::NativeNew", managedbindings::entity_new);
		mono_add_internal_call("leopph.Entity::NativeGetWorldPos", managedbindings::entity_get_world_position);
		mono_add_internal_call("leopph.Entity::NativeSetWorldPos", managedbindings::set_entity_world_position);
		mono_add_internal_call("leopph.Entity::NativeGetLocalPos", managedbindings::get_entity_local_position);
		mono_add_internal_call("leopph.Entity::NativeSetLocalPos", managedbindings::set_entity_local_position);
		mono_add_internal_call("leopph.Entity::NativeGetWorldRot", managedbindings::get_entity_world_rotation);
		mono_add_internal_call("leopph.Entity::NativeSetWorldRot", managedbindings::set_entity_world_rotation);
		mono_add_internal_call("leopph.Entity::NativeGetLocalRot", managedbindings::get_entity_local_rotation);
		mono_add_internal_call("leopph.Entity::NativeSetLocalRot", managedbindings::set_entity_local_rotation);
		mono_add_internal_call("leopph.Entity::NativeGetWorldScale", managedbindings::get_entity_world_scale);
		mono_add_internal_call("leopph.Entity::NativeSetWorldScale", managedbindings::set_entity_world_scale);
		mono_add_internal_call("leopph.Entity::NativeGetLocalScale", managedbindings::get_entity_local_scale);
		mono_add_internal_call("leopph.Entity::NativeSetLocalScale", managedbindings::set_entity_local_scale);
		mono_add_internal_call("leopph.Entity::NativeTranslateVector", managedbindings::translate_entity_from_vector);
		mono_add_internal_call("leopph.Entity::NativeTranslate", managedbindings::translate_entity);
		mono_add_internal_call("leopph.Entity::NativeRotate", managedbindings::rotate_entity);
		mono_add_internal_call("leopph.Entity::NativeRotateAngleAxis", managedbindings::rotate_entity_angle_axis);
		mono_add_internal_call("leopph.Entity::NativeRescaleVector", managedbindings::rescale_entity_from_vector);
		mono_add_internal_call("leopph.Entity::NativeRescale", managedbindings::rescale_entity);
		mono_add_internal_call("leopph.Entity::NativeGetRightAxis", managedbindings::get_entity_right_axis);
		mono_add_internal_call("leopph.Entity::NativeGetUpAxis", managedbindings::get_entity_up_axis);
		mono_add_internal_call("leopph.Entity::NativeGetForwardtAxis", managedbindings::get_entity_forward_axis);
		mono_add_internal_call("leopph.Entity::NativeGetParentHandle", managedbindings::get_entity_parent_handle);
		mono_add_internal_call("leopph.Entity::NativeSetParent", managedbindings::set_entity_parent);
		mono_add_internal_call("leopph.Entity::NativeGetChildCount", managedbindings::get_entity_child_count);
		mono_add_internal_call("leopph.Entity::NativeGetChildHandle", managedbindings::get_entity_child_handle);
		mono_add_internal_call("leopph.Entity::InternalCreateComponent", managedbindings::create_component);

		mono_add_internal_call("leopph.Component::InternalGetEntityHandle", managedbindings::component_get_entity_handle);

		char* exePath;
		_get_pgmptr(&exePath);
		std::filesystem::path managedLibPath{ exePath };
		managedLibPath = managedLibPath.remove_filename();
		managedLibPath /= "LeopphRuntimeManaged.dll";

		MonoAssembly* assembly = mono_domain_assembly_open(gDomain, managedLibPath.string().c_str());
		assert(assembly);

		auto const image = mono_assembly_get_image(assembly);
		assert(image);

		MonoClass* testClass = mono_class_from_name(image, "", "Test");
		MonoMethod* doTestMethod = mono_class_get_method_from_name(testClass, "DoTest", 0);
		mono_runtime_invoke(doTestMethod, nullptr, nullptr, nullptr);

		return true;
	}


	void cleanup_managed_runtime()
	{
		destroy_all_maos();
		mono_jit_cleanup(gDomain);
	}
}