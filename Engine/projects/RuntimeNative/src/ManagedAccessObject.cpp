#include "ManagedAccessObject.hpp"

#include "ManagedRuntime.hpp"

#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/class.h>

#include <unordered_map>
#include <memory>


namespace leopph
{
	u64 ManagedAccessObject::sNextId{ 1 };


	void ManagedAccessObject::SetManagedObject(MonoObject* const managedObject)
	{
		u64 idData{ id };
		void* ptrData{ this };

		auto const managedClass = mono_object_get_class(managedObject);

		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_id"), &idData);
		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_ptr"), &ptrData);

		mGcHandle = mono_gchandle_new(managedObject, false);
	}


	MonoObject* ManagedAccessObject::CreateManagedObject(MonoClass* const klass)
	{
		if (mGcHandle)
		{
			return mono_gchandle_get_target(*mGcHandle);
		}

		auto const object = mono_object_new(GetManagedDomain(), klass);
		SetManagedObject(object);
		return object;
	}


	MonoObject* ManagedAccessObject::CreateManagedObject(std::string_view const classNamespace, std::string_view const className)
	{
		return CreateManagedObject(mono_class_from_name(GetManagedImage(), classNamespace.data(), className.data()));
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

			u64 idData{ 0 };
			nullptr_t ptrData{ nullptr };

			mono_field_set_value(object, mono_class_get_field_from_name(klass, "_id"), &idData);
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
		auto GetManagedAccessObjectGuid(MonoObject* const nativeWrapper) -> GUID
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<ManagedAccessObject*>(nativeWrapper)->GetGuid();
		}
	}
}