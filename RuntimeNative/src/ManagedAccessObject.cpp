#include "ManagedAccessObject.hpp"

#include "ManagedRuntime.hpp"
#include "Systems.hpp"

#include <mono/metadata/object.h>
#include <mono/metadata/class.h>


namespace leopph {
	auto ManagedAccessObject::AcquireManagedObject(MonoObject* const managedObject) -> void {
		if (HasManagedObject()) {
			throw std::logic_error{ "ManagedAccessObject failed to acquire managed object, because it already has a managed counterpart." };
		}

		void* ptrData{ this };
		auto const managedClass = mono_object_get_class(managedObject);
		mono_field_set_value(managedObject, mono_class_get_field_from_name(managedClass, "_ptr"), &ptrData);
		mGcHandle = mono_gchandle_new(managedObject, false);
	}


	auto ManagedAccessObject::HasManagedObject() const noexcept -> bool {
		return mGcHandle.has_value();
	}


	auto ManagedAccessObject::CreateManagedObject(MonoClass* const klass) -> void {
		if (HasManagedObject()) {
			throw std::logic_error{ "Failed to create managed object for ManagedAccessObject, because the object already has a managed counterpart." };
		}

		AcquireManagedObject(mono_object_new(gManagedRuntime.GetManagedDomain(), klass));
	}


	auto ManagedAccessObject::CreateManagedObject(std::string_view const classNamespace, std::string_view const className) -> void {
		CreateManagedObject(mono_class_from_name(gManagedRuntime.GetManagedImage(), classNamespace.data(), className.data()));
	}


	auto ManagedAccessObject::GetManagedObject() const -> MonoObject* {
		if (!HasManagedObject()) {
			throw std::runtime_error{ "Failed to get ManagedAccessObject's managed object, because it does not exist." };
		}

		return mono_gchandle_get_target(*mGcHandle);
	}


	ManagedAccessObject::~ManagedAccessObject() {
		if (HasManagedObject()) {
			auto const object = mono_gchandle_get_target(*mGcHandle);
			auto const klass = mono_object_get_class(object);

			auto ptrData{ nullptr };
			mono_field_set_value(object, mono_class_get_field_from_name(klass, "_ptr"), &ptrData);

			mono_gchandle_free(*mGcHandle);
		}
	}


	auto ManagedAccessObject::GetNativePtrFromManagedObject(MonoObject* const managedObject) -> void* {
		auto const klass = mono_object_get_class(managedObject);
		auto const field = mono_class_get_field_from_name(klass, "_ptr");
		void* ptrData;
		mono_field_get_value(managedObject, field, &ptrData);
		return ptrData;
	}


	namespace managedbindings {
		auto GetManagedAccessObjectGuid(MonoObject* const nativeWrapper) -> Guid {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<ManagedAccessObject*>(nativeWrapper)->GetGuid();
		}
	}
}
