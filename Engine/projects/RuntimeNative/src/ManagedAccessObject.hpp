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
		std::optional<u32> mGcHandle;

	protected:
		auto CreateManagedObject(MonoClass* klass) -> void;
		auto CreateManagedObject(std::string_view classNamespace, std::string_view className) -> void;
		auto AcquireManagedObject(MonoObject* managedObject) -> void;

	public:
		[[nodiscard]] LEOPPHAPI auto HasManagedObject() const noexcept -> bool;
		[[nodiscard]] LEOPPHAPI auto GetManagedObject() const -> MonoObject*;
		LEOPPHAPI virtual auto CreateManagedObject() -> void = 0;

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
