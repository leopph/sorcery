#include "ManagedAccessObject.hpp"

#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>

#include <unordered_map>
#include <memory>

namespace leopph
{
	static std::unordered_map<u64, std::unique_ptr<ManagedAccessObject>> gAllObjects;

	u64 ManagedAccessObject::sNextId{ 1 };


	ManagedAccessObject::ManagedAccessObject(u64 const managedObjectHandle) :
		managedObjectHandle{ managedObjectHandle }
	{}


	ManagedAccessObject::~ManagedAccessObject()
	{
		MonoObject* managedObject = mono_gchandle_get_target(static_cast<u32>(managedObjectHandle));
		MonoClass* managedClass = mono_object_get_class(managedObject);
		u64 idData{ 0 };
		nullptr_t ptrData{nullptr};
		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_id"), &idData);
		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_ptr"), &ptrData);
		mono_gchandle_free(static_cast<u32>(managedObjectHandle));
	}


	void store_mao(ManagedAccessObject* const mao)
	{
		gAllObjects[mao->id] = std::unique_ptr<ManagedAccessObject>{ mao };
	}


	void destroy_mao(u64 const id)
	{
		gAllObjects[id].reset();
	}


	ManagedAccessObject* get_mao_by_id(u64 const id)
	{
		return gAllObjects[id].get();
	}


	void destroy_all_maos()
	{
		gAllObjects.clear();
	}
}