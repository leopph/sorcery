#pragma once

#include "Core.hpp"
#include "Object.hpp"

#include <string_view>
#include <optional>
#include <concepts>
#include <vector>


using MonoObject = struct _MonoObject;
using MonoClass = struct _MonoClass;


namespace leopph
{
	class ManagedAccessObject : public Object
	{
	private:
		std::optional<u32> mGcHandle;

	public:
		[[nodiscard]] LEOPPHAPI MonoObject* GetManagedObject() const;
		LEOPPHAPI void SetManagedObject(MonoObject* managedObject);
		LEOPPHAPI MonoObject* CreateManagedObject(MonoClass* klass);
		LEOPPHAPI MonoObject* CreateManagedObject(std::string_view classNamespace, std::string_view className);

		LEOPPHAPI virtual ~ManagedAccessObject();

		[[nodiscard]] static void* GetNativePtrFromManagedObject(MonoObject* managedObject);

		template<typename T> requires std::derived_from<std::remove_pointer_t<T>, ManagedAccessObject>
		[[nodiscard]] static T GetNativePtrFromManagedObjectAs(MonoObject* managedObject)
		{
			return static_cast<T>(GetNativePtrFromManagedObject(managedObject));
		}
	}; 


	namespace managedbindings {
		auto GetManagedAccessObjectGuid(MonoObject* nativeWrapper) -> Guid;
	}
}