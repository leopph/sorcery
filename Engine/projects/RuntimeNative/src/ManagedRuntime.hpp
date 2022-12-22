#pragma once

#include "Core.hpp"

#include <span>
#include <string_view>
#include <vector>

using MonoImage = struct _MonoImage;
using MonoDomain = struct _MonoDomain;
using MonoMethod = struct _MonoMethod;
using MonoReflectionField = struct _MonoReflectionField;
using MonoReflectionProperty = struct _MonoReflectionProperty;
using MonoObject = struct _MonoObject;
using MonoClass = struct _MonoClass;
using MonoAppContext = struct _MonoAppContext;
using MonoArray = struct _MonoArray;
using MonoReflectionType = struct _MonoReflectionType;
using MonoType = struct _MonoType;

namespace leopph {
	class ManagedRuntime {
	private:
		MonoDomain* mDomain;
		MonoImage* mImage;

		std::vector<MonoClass*> mComponentClasses;

		MonoMethod* mFieldSerializeTestMethod;
		MonoMethod* mPropertySerializeTestMethod;
		MonoMethod* mGetEnumValuesMethod;
		MonoMethod* mPrimitiveTestMethod;
		MonoMethod* mParsePropertyValueMethod;
		MonoMethod* mParseFieldValueMethod;
		MonoMethod* mEnumToUnderlyingValueMethod;
		MonoMethod* mParseEnumValueMethod;

	public:
		ManagedRuntime() noexcept = default;
		~ManagedRuntime() noexcept = default;

		LEOPPHAPI auto StartUp() -> void;
		LEOPPHAPI auto ShutDown() noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto GetManagedImage() const -> MonoImage*;
		[[nodiscard]] LEOPPHAPI auto GetManagedDomain() const -> MonoDomain*;
		[[nodiscard]] LEOPPHAPI auto ShouldSerialize(MonoReflectionField* field) const -> bool;
		[[nodiscard]] LEOPPHAPI auto ShouldSerialize(MonoReflectionProperty* prop) const -> bool;
		[[nodiscard]] LEOPPHAPI auto GetEnumValues(MonoReflectionType* enumType) const -> MonoArray*;
		[[nodiscard]] LEOPPHAPI auto GetComponentClasses() -> std::span<MonoClass* const>;
		[[nodiscard]] LEOPPHAPI auto IsTypePrimitive(MonoReflectionType* refType) const -> bool;
		[[nodiscard]] LEOPPHAPI auto ParseValue(MonoReflectionField* field, std::string_view str) const -> MonoObject*;
		[[nodiscard]] LEOPPHAPI auto ParseValue(MonoReflectionProperty* property, std::string_view str) const -> MonoObject*;
		[[nodiscard]] LEOPPHAPI auto EnumToUnderlyingType(MonoReflectionType* enumType, MonoObject* enumValue) const -> MonoObject*;
		[[nodiscard]] LEOPPHAPI auto ParseEnumValue(MonoReflectionType* enumType, std::string_view str) const -> MonoObject*;
	};
}
