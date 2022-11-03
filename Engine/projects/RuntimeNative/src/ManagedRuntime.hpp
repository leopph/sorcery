#pragma once

#include "Core.hpp"

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
	LEOPPHAPI [[nodiscard]] bool IsFieldExposed(MonoReflectionField* field);
	LEOPPHAPI [[nodiscard]] bool IsPropertyExposed(MonoReflectionProperty* prop);
	LEOPPHAPI [[nodiscard]] MonoArray* GetEnumValues(MonoType* enumType);
}