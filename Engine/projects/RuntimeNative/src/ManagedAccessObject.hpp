#pragma once

#include "Core.hpp"

#include <string_view>
#include <optional>
#include <concepts>


using MonoObject = struct _MonoObject;
using MonoClass = struct _MonoClass;


namespace leopph
{
	class ManagedAccessObject
	{
	private:
		static u64 sNextId;
		std::optional<u32> mGcHandle;

	public:
		u64 const id{ sNextId++ };

		LEOPPHAPI [[nodiscard]] MonoObject* GetManagedObject() const;
		LEOPPHAPI void SetManagedObject(MonoObject* managedObject);
		LEOPPHAPI MonoObject* CreateManagedObject(MonoClass* klass);
		LEOPPHAPI MonoObject* CreateManagedObject(std::string_view classNamespace, std::string_view className);

		virtual ~ManagedAccessObject();

		static [[nodiscard]] void* GetNativePtrFromManagedObject(MonoObject* managedObject);

		template<typename T> requires std::derived_from<std::remove_pointer_t<T>, ManagedAccessObject>
		static [[nodiscard]] T GetNativePtrFromManagedObjectAs(MonoObject* managedObject)
		{
			return static_cast<T>(GetNativePtrFromManagedObject(managedObject));
		}
	}; 
}