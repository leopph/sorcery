#include "ManagedAccessObject.hpp"

#include "ManagedRuntime.hpp"
#include "Systems.hpp"

#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/class.h>

#include <unordered_map>
#include <memory>


namespace leopph
{
	void ManagedAccessObject::SetManagedObject(MonoObject* const managedObject)
	{
		void* ptrData{ this };

		auto const managedClass = mono_object_get_class(managedObject);
		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_ptr"), &ptrData);
		mGcHandle = mono_gchandle_new(managedObject, false);
	}


	MonoObject* ManagedAccessObject::CreateManagedObject(MonoClass* const klass)
	{
		if (mGcHandle)
		{
			return mono_gchandle_get_target(*mGcHandle);
		}

		auto const object = mono_object_new(gManagedRuntime.GetManagedDomain(), klass);
		SetManagedObject(object);
		return object;
	}


	MonoObject* ManagedAccessObject::CreateManagedObject(std::string_view const classNamespace, std::string_view const className)
	{
		return CreateManagedObject(mono_class_from_name(gManagedRuntime.GetManagedImage(), classNamespace.data(), className.data()));
	}


	MonoObject* ManagedAccessObject::GetManagedObject() const
	{
		if (mGcHandle)
		{
			return mono_gchandle_get_target(*mGcHandle);
		}

		return nullptr;
	}


	ManagedAccessObject::~ManagedAccessObject()
	{
		if (mGcHandle)
		{
			auto const object = mono_gchandle_get_target(*mGcHandle);
			auto const klass = mono_object_get_class(object);

			nullptr_t ptrData{ nullptr };
			mono_field_set_value(object, mono_class_get_field_from_name(klass, "_ptr"), &ptrData);

			mono_gchandle_free(*mGcHandle);
		}
	}


	void* ManagedAccessObject::GetNativePtrFromManagedObject(MonoObject* const managedObject)
	{
		auto const klass = mono_object_get_class(managedObject);
		auto const field = mono_class_get_field_from_name(klass, "_ptr");
		void* ptrData;
		mono_field_get_value(managedObject, field, &ptrData);
		return ptrData;
	}


	namespace managedbindings {
		auto GetManagedAccessObjectGuid(MonoObject* const nativeWrapper) -> Guid
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<ManagedAccessObject*>(nativeWrapper)->GetGuid();
		}
	}
}