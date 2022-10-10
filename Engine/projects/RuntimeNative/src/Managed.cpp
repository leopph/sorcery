#include "Managed.hpp"

#include "Input.hpp"
#include "Entity.hpp"
#include "Time.hpp"

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
		MonoDomain* gAppDomain;
		MonoAssembly* gRuntimeAssembly;
		MonoImage* gRuntimeImage;
		MonoClass* gEntityClass;
	}


	std::vector<ManagedEntityInstance> managedEntityInstances;


	void initialize_managed_runtime()
	{
		gAppDomain = mono_jit_init("leopph");
		assert(gAppDomain);

		mono_add_internal_call("leopph.Input::GetKeyDown", detail::get_key_down);
		mono_add_internal_call("leopph.Input::GetKey", detail::get_key);
		mono_add_internal_call("leopph.Input::GetKeyUp", detail::get_key_up);

		mono_add_internal_call("leopph.Entity::NativeNewEntity", detail::new_entity);
		mono_add_internal_call("leopph.Entity::NativeIsEntityAlive", detail::is_entity_alive);
		mono_add_internal_call("leopph.Entity::NativeDeleteEntity", detail::delete_entity);

		mono_add_internal_call("leopph.Entity::NativeGetWorldPos", detail::get_entity_world_position);
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

		mono_add_internal_call("leopph.Entity::NativeGetParentId", detail::get_entity_parent_id);
		mono_add_internal_call("leopph.Entity::NativeSetParent", detail::set_entity_parent);

		mono_add_internal_call("leopph.Entity::NativeGetChildCount", detail::get_entity_child_count);
		mono_add_internal_call("leopph.Entity::NativeGetChildId", detail::get_entity_child_id);

		mono_add_internal_call("leopph.Time::get_FullTime", detail::get_full_time);
		mono_add_internal_call("leopph.Time::get_FrameTime", detail::get_frame_time);

		gRuntimeAssembly = mono_domain_assembly_open(gAppDomain, "LeopphRuntimeManaged.dll");
		assert(gRuntimeAssembly);

		gRuntimeImage = mono_assembly_get_image(gRuntimeAssembly);
		assert(gRuntimeImage);

		gEntityClass = mono_class_from_name(gRuntimeImage, "leopph", "Entity");
		assert(gEntityClass);

		MonoTableInfo const* const tableInfo = mono_image_get_table_info(gRuntimeImage, MONO_TABLE_TYPEDEF);
		int const numRows = mono_table_info_get_rows(tableInfo);

		for (int i = 0; i < numRows; i++)
		{
			u32 cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(tableInfo, i, cols, MONO_TYPEDEF_SIZE);

			char const* const className = mono_metadata_string_heap(gRuntimeImage, cols[MONO_TYPEDEF_NAME]);
			assert(className);

			char const* const classNamespace = mono_metadata_string_heap(gRuntimeImage, cols[MONO_TYPEDEF_NAMESPACE]);
			assert(classNamespace);

			MonoClass* const clasz = mono_class_from_name(gRuntimeImage, classNamespace, className);
			assert(clasz);

			if (clasz != gEntityClass && mono_class_is_subclass_of(clasz, gEntityClass, false))
			{
				managedEntityInstances.emplace_back(clasz);
			}
		}
	}


	ManagedEntityInstance::ManagedEntityInstance(MonoClass* const monoClass) :
		mGcHandle{mono_gchandle_new(mono_object_new(mono_domain_get(), monoClass), false)},
		mTickMethod{mono_class_get_method_from_name(monoClass, "Tick", 0)}
	{
		assert(mGcHandle);

		if (MonoMethod* defaultCtor = mono_class_get_method_from_name(monoClass, ".ctor", 0))
		{
			MonoObject* exception;
			mono_runtime_invoke(defaultCtor, mono_gchandle_get_target(mGcHandle), nullptr, &exception);
			check_and_handle_exception(exception);
		}
	}


	ManagedEntityInstance::ManagedEntityInstance(ManagedEntityInstance&& other) noexcept :
		mGcHandle{other.mGcHandle},
		mTickMethod{other.mTickMethod}
	{
		other.mGcHandle = 0;
		other.mTickMethod = nullptr;
	}


	ManagedEntityInstance::~ManagedEntityInstance()
	{
		if (mGcHandle)
		{
			mono_gchandle_free(mGcHandle);
		}
	}


	void ManagedEntityInstance::tick() const
	{
		if (!mTickMethod)
		{
			return;
		}

		MonoObject* exception;
		mono_runtime_invoke(mTickMethod, mono_gchandle_get_target(mGcHandle), nullptr, &exception);
		check_and_handle_exception(exception);
	}


	void ManagedEntityInstance::check_and_handle_exception(MonoObject* const exception)
	{
		if (exception)
		{
			mono_print_unhandled_exception(exception);
		}
	}
}