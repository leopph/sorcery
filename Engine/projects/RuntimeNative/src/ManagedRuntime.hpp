#pragma once

#include "Core.hpp"

#include <span>
#include <string_view>

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

namespace leopph
{
	[[nodiscard]] LEOPPHAPI bool initialize_managed_runtime();
	LEOPPHAPI void cleanup_managed_runtime();	
	[[nodiscard]] LEOPPHAPI MonoImage* GetManagedImage();
	[[nodiscard]] LEOPPHAPI MonoDomain* GetManagedDomain();
	[[nodiscard]] LEOPPHAPI bool ShouldSerialize(MonoReflectionField* field);
	[[nodiscard]] LEOPPHAPI bool ShouldSerialize(MonoReflectionProperty* prop);
	[[nodiscard]] LEOPPHAPI MonoArray* GetEnumValues(MonoReflectionType* enumType);
	[[nodiscard]] LEOPPHAPI std::span<MonoClass* const> GetComponentClasses();
	[[nodiscard]] LEOPPHAPI bool IsTypePrimitive(MonoReflectionType* refType);
	[[nodiscard]] LEOPPHAPI MonoObject* ParseValue(MonoReflectionField* field, std::string_view str);
	[[nodiscard]] LEOPPHAPI MonoObject* ParseValue(MonoReflectionProperty* property, std::string_view str);
	[[nodiscard]] LEOPPHAPI MonoObject* EnumToUnderlyingType(MonoReflectionType* enumType, MonoObject* enumValue);
	[[nodiscard]] LEOPPHAPI MonoObject* ParseEnumValue(MonoReflectionType* enumType, std::string_view str);
}