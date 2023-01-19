using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;

namespace leopph {
	internal static class NativeHelpers {
		private static readonly object[] s_ObjectArrayTwoElems = new object[2];
		private static readonly Type[] s_TypeArrayTwoElems = new Type[2];
		private static readonly Type s_stringType = typeof(string);
		private static readonly Type s_cultureInfoType = typeof(CultureInfo);
		private static readonly Type s_exposeAttributeType = typeof(ExposeAttribute);
		private static readonly Type s_doNotExposeAttributeType = typeof(DoNotExposeAttribute);
		private static readonly Assembly s_thisAssembly = typeof(NativeHelpers).Assembly;
		private static readonly Type s_listType = typeof(List<>);


		internal static bool ShouldSerializeField(FieldInfo field) {
			if (field.IsStatic) {
				return false;
			}

			var typeToCheck = field.FieldType;

			if (field.FieldType.IsArray) {
				typeToCheck = typeToCheck.GetElementType();
			}
			else if (field.FieldType.IsGenericType && field.FieldType.GetGenericTypeDefinition() == s_listType) {
				typeToCheck = field.FieldType.GetGenericArguments()[0];
			}

			var typeIsExposed = typeToCheck.IsPrimitive || typeToCheck.Assembly == s_thisAssembly;

			foreach (var attr in field.FieldType.CustomAttributes) {
				if (attr.AttributeType == s_exposeAttributeType) {
					typeIsExposed = true;
					break;
				}
			}

			if (!typeIsExposed) {
				return false;
			}

			foreach (var attr in field.CustomAttributes) {
				if (attr.AttributeType == s_exposeAttributeType) {
					return true;
				}
				if (attr.AttributeType == s_doNotExposeAttributeType) {
					return false;
				}
			}

			if (field.IsPublic) {
				return true;
			}

			return false;
		}


		internal static bool ShouldSerializeProperty(PropertyInfo property) {
			if (property.GetMethod is null || property.GetMethod.IsStatic || property.GetMethod.GetParameters().Length != 0 || property.SetMethod is null || property.SetMethod.IsStatic) {
				return false;
			}

			var typeToCheck = property.GetMethod.ReturnType;

			if (property.GetMethod.ReturnType.IsArray) {
				typeToCheck = typeToCheck.GetElementType();
			}
			else if (property.GetMethod.ReturnType.IsGenericType && property.GetMethod.ReturnType.GetGenericTypeDefinition() == s_listType) {
				typeToCheck = property.GetMethod.ReturnType.GetGenericArguments()[0];
			}

			var typeIsExposed = typeToCheck.IsPrimitive || typeToCheck.Assembly == s_thisAssembly;

			foreach (var attr in property.GetMethod.ReturnType.CustomAttributes) {
				if (attr.AttributeType == s_exposeAttributeType) {
					typeIsExposed = true;
					break;
				}
			}

			if (!typeIsExposed) {
				return false;
			}

			foreach (var attr in property.CustomAttributes) {
				if (attr.AttributeType == s_exposeAttributeType) {
					return true;
				}
				if (attr.AttributeType == s_doNotExposeAttributeType) {
					return false;
				}
			}

			if (property.GetMethod.IsPublic && property.SetMethod.IsPublic) {
				return true;
			}

			return false;
		}


		internal static Array GetEnumValues(Type enumType) {
			return Enum.GetValues(enumType);
		}


		internal static bool IsPrimitive(Type type) {
			return type.IsPrimitive;
		}


		internal static object? ParseFieldValue(FieldInfo field, string value) {
			s_ObjectArrayTwoElems[0] = value;
			s_ObjectArrayTwoElems[1] = CultureInfo.InvariantCulture;
			s_TypeArrayTwoElems[0] = s_stringType;
			s_TypeArrayTwoElems[1] = s_cultureInfoType;

			try {
				var parseMethod = field.FieldType.GetMethod("Parse", s_TypeArrayTwoElems);
				var parsedValue = parseMethod?.Invoke(null, s_ObjectArrayTwoElems);
				return parsedValue;
			}
			catch {
				return null;
			}
		}


		internal static object? ParsePropertyValue(PropertyInfo property, string value) {
			s_ObjectArrayTwoElems[0] = value;
			s_ObjectArrayTwoElems[1] = CultureInfo.InvariantCulture;
			s_TypeArrayTwoElems[0] = s_stringType;
			s_TypeArrayTwoElems[1] = s_cultureInfoType;

			try {
				var parseMethod = property.GetMethod.ReturnType.GetMethod("Parse", s_TypeArrayTwoElems);
				var parsedValue = parseMethod?.Invoke(null, s_ObjectArrayTwoElems);
				return parsedValue;
			}
			catch {
				return null;
			}
		}


		internal static object? ParseEnumValue(Type enumType, string value) {
			try {
				return Enum.Parse(enumType, value);
			}
			catch {
				return null;
			}
		}


		internal static object? EnumToUnderlyingType(Type enumType, object enumValue) {
			try {
				var underlyingType = Enum.GetUnderlyingType(enumType);
				var underlyingValue = Convert.ChangeType(enumValue, underlyingType);
				return underlyingValue;
			}
			catch {
				return null;
			}
		}
	}
}
