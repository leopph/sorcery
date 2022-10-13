#include "Managed.hpp"

#include "Behavior.hpp"
#include "Cube.hpp"
#include "Entity.hpp"
#include "Input.hpp"
#include "Time.hpp"
#include "Window.hpp"
#include "ManagedAccessObject.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <cstdio>
#include <cassert>
#include <vector>


namespace leopph
{
	namespace
	{
		MonoDomain* gDomain;
		MonoAssembly* gRuntimeAssembly;
		MonoImage* gRuntimeImage;
	}


	void initialize_managed_runtime()
	{
		gDomain = mono_jit_init("leopph");
		assert(gDomain);

		mono_add_internal_call("leopph.Input::GetKeyDown", detail::get_key_down);
		mono_add_internal_call("leopph.Input::GetKey", detail::get_key);
		mono_add_internal_call("leopph.Input::GetKeyUp", detail::get_key_up);

		mono_add_internal_call("leopph.NativeWrapper::InternalDestroyMAO", destroy_mao);

		mono_add_internal_call("leopph.Entity::NativeNew", detail::entity_new);

		mono_add_internal_call("leopph.Entity::NativeGetWorldPos", detail::entity_get_world_position);
		mono_add_internal_call("leopph.Entity::NativeSetWorldPos", detail::set_entity_world_position);
		mono_add_internal_call("leopph.Entity::NativeGetLocalPos", detail::get_entity_local_position);
		mono_add_internal_call("leopph.Entity::NativeSetLocalPos", detail::set_entity_local_position);

		mono_add_internal_call("leopph.Entity::NativeGetWorldRot", detail::get_entity_world_rotation);
		mono_add_internal_call("leopph.Entity::NativeSetWorldRot", detail::set_entity_world_rotation);
		mono_add_internal_call("leopph.Entity::NativeGetLocalRot", detail::get_entity_local_rotation);
		mono_add_internal_call("leopph.Entity::NativeSetLocalRot", detail::set_entity_local_rotation);

		mono_add_internal_call("leopph.Entity::NativeGetWorldScale", detail::get_entity_world_scale);
		mono_add_internal_call("leopph.Entity::NativeSetWorldScale", detail::set_entity_world_scale);
		mono_add_internal_call("leopph.Entity::NativeGetLocalScale", detail::get_entity_local_scale);
		mono_add_internal_call("leopph.Entity::NativeSetLocalScale", detail::set_entity_local_scale);

		mono_add_internal_call("leopph.Entity::NativeTranslateVector", detail::translate_entity_from_vector);
		mono_add_internal_call("leopph.Entity::NativeTranslate", detail::translate_entity);

		mono_add_internal_call("leopph.Entity::NativeRotate", detail::rotate_entity);
		mono_add_internal_call("leopph.Entity::NativeRotateAngleAxis", detail::rotate_entity_angle_axis);

		mono_add_internal_call("leopph.Entity::NativeRescaleVector", detail::rescale_entity_from_vector);
		mono_add_internal_call("leopph.Entity::NativeRescale", detail::rescale_entity);

		mono_add_internal_call("leopph.Entity::NativeGetRightAxis", detail::get_entity_right_axis);
		mono_add_internal_call("leopph.Entity::NativeGetUpAxis", detail::get_entity_up_axis);
		mono_add_internal_call("leopph.Entity::NativeGetForwardtAxis", detail::get_entity_forward_axis);

		mono_add_internal_call("leopph.Entity::NativeGetParentHandle", detail::get_entity_parent_handle);
		mono_add_internal_call("leopph.Entity::NativeSetParent", detail::set_entity_parent);

		mono_add_internal_call("leopph.Entity::NativeGetChildCount", detail::get_entity_child_count);
		mono_add_internal_call("leopph.Entity::NativeGetChildHandle", detail::get_entity_child_handle);

		mono_add_internal_call("leopph.Time::get_FullTime", detail::get_full_time);
		mono_add_internal_call("leopph.Time::get_FrameTime", detail::get_frame_time);

		mono_add_internal_call("Cube::InternalAddPos", add_cube_pos);
		mono_add_internal_call("Cube::InternalUpdatePos", update_cube_pos);

		mono_add_internal_call("leopph.Entity::InternalCreateBehavior", detail::behavior_new);

		mono_add_internal_call("leopph.Behavior::InternalGetEntityHandle", detail::behavior_get_entity_handle);

		mono_add_internal_call("leopph.Window::InternalGetCurrentClientAreaSize", detail::get_window_current_client_area_size);
		mono_add_internal_call("leopph.Window::InternalGetWindowedClientAreaSize", detail::get_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::InternalSetWindowedClientAreaSize", detail::set_window_windowed_client_area_size);
		mono_add_internal_call("leopph.Window::InternalIsBorderless", detail::is_window_borderless);
		mono_add_internal_call("leopph.Window::InternalSetBorderless", detail::set_window_borderless);
		mono_add_internal_call("leopph.Window::InternalIsMinimizingOnBorderlessFocusLoss", detail::is_window_minimizing_on_borderless_focus_loss);
		mono_add_internal_call("leopph.Window::InternalSetMinimizeOnBorderlessFocusLoss", detail::set_window_minimize_on_borderless_focus_loss);

		gRuntimeAssembly = mono_domain_assembly_open(gDomain, "LeopphRuntimeManaged.dll");
		assert(gRuntimeAssembly);

		gRuntimeImage = mono_assembly_get_image(gRuntimeAssembly);
		assert(gRuntimeImage);

		MonoClass* testClass = mono_class_from_name(gRuntimeImage, "", "Test");
		MonoMethod* doTestMethod = mono_class_get_method_from_name(testClass, "DoTest", 0);

		MonoObject* exception;
		mono_runtime_invoke(doTestMethod, nullptr, nullptr, &exception);

		if (exception)
		{
			mono_print_unhandled_exception(exception);
		}
	}


	void cleanup_managed_runtime()
	{
		destroy_all_maos();
		mono_jit_cleanup(gDomain);
	}
}