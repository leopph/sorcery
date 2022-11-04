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
	LEOPPHAPI [[nodiscard]] bool initialize_managed_runtime();
	LEOPPHAPI void cleanup_managed_runtime();	
	LEOPPHAPI [[nodiscard]] MonoImage* GetManagedImage();
	LEOPPHAPI [[nodiscard]] MonoDomain* GetManagedDomain();
	LEOPPHAPI [[nodiscard]] bool ShouldSerialize(MonoReflectionField* field);
	LEOPPHAPI [[nodiscard]] bool ShouldSerialize(MonoReflectionProperty* prop);
	LEOPPHAPI [[nodiscard]] MonoArray* GetEnumValues(MonoType* enumType);
	LEOPPHAPI [[nodiscard]] std::span<MonoClass* const> GetComponentClasses();
	LEOPPHAPI [[nodiscard]] bool IsTypePrimitive(MonoReflectionType* refType);
	LEOPPHAPI [[nodiscard]] MonoObject* ParseValue(MonoReflectionField* field, std::string_view str);
	LEOPPHAPI [[nodiscard]] MonoObject* ParseValue(MonoReflectionProperty* property, std::string_view str);
	LEOPPHAPI [[nodiscard]] MonoObject* EnumToUnderlyingType(MonoReflectionType* enumType, MonoObject* enumValue);
	LEOPPHAPI [[nodiscard]] MonoObject* ParseEnumValue(MonoReflectionType* enumType, std::string_view str);
}