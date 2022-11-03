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

	protected:
		void SetManagedObject(MonoObject* managedObject);
		MonoObject* CreateManagedObject(MonoClass* klass);
		MonoObject* CreateManagedObject(std::string_view classNamespace, std::string_view className);

	public:
		u64 const id{ sNextId++ };

		LEOPPHAPI MonoObject* GetManagedObject() const;

		virtual ~ManagedAccessObject();

		static [[nodiscard]] void* GetNativePtrFromManagedObject(MonoObject* managedObject);

		template<typename T> requires std::derived_from<std::remove_pointer_t<T>, ManagedAccessObject>
		static [[nodiscard]] T GetNativePtrFromManagedObjectAs(MonoObject* managedObject)
		{
			return static_cast<T>(GetNativePtrFromManagedObject(managedObject));
		}
	}; 
}