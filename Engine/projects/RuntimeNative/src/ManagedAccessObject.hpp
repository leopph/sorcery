#pragma once

#include "Core.hpp"
#include "Object.hpp"

#include <string_view>
#include <optional>
#include <concepts>
#include <vector>


using MonoObject = struct _MonoObject;
using MonoClass = struct _MonoClass;


namespace leopph {
	class ManagedAccessObject : public Object {
	private:
		std::optional<u32> mGcHandle;

	public:
		[[nodiscard]] LEOPPHAPI auto GetManagedObject() const -> MonoObject*;
		LEOPPHAPI auto SetManagedObject(MonoObject* managedObject) -> void;
		LEOPPHAPI auto CreateManagedObject(MonoClass* klass) -> MonoObject*;
		LEOPPHAPI auto CreateManagedObject(std::string_view classNamespace, std::string_view className) -> MonoObject*;
		LEOPPHAPI virtual auto CreateManagedObject() -> MonoObject* = 0;

		LEOPPHAPI ~ManagedAccessObject() override;

		[[nodiscard]] static auto GetNativePtrFromManagedObject(MonoObject* managedObject) -> void*;

		template<typename T> requires std::derived_from<std::remove_pointer_t<T>, ManagedAccessObject>
		[[nodiscard]] static auto GetNativePtrFromManagedObjectAs(MonoObject* managedObject) -> T {
			return static_cast<T>(GetNativePtrFromManagedObject(managedObject));
		}
	};


	namespace managedbindings {
		auto GetManagedAccessObjectGuid(MonoObject* nativeWrapper) -> Guid;
	}
}
