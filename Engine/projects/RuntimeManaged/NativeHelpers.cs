using System;
using System.Globalization;
using System.Reflection;

namespace leopph
{
	internal static class NativeHelpers
	{
		private static readonly object[] s_ObjectArrayTwoElems = new object[2];
		private static readonly Type[] s_TypeArrayTwoElems = new Type[2];
		private static readonly Type s_stringType = typeof(string);
		private static readonly Type s_cultureInfoType = typeof(CultureInfo);

		internal static bool ShouldSerializeField(FieldInfo field)
		{
			// First check for reasons to NOT serialize

			if (field.IsStatic)
			{
				return false;
			}

			// Prohibiting attribute is stronger
			foreach (var attr in field.CustomAttributes)
			{

				if (attr.AttributeType == typeof(DoNotExposeAttribute))
				{
					return false;
				}
			}

			if (field.IsPublic)
			{
				return true;
			}

			foreach (var attr in field.CustomAttributes)
			{
				if (attr.AttributeType == typeof(ExposeAttribute))
				{
					return true;
				}
			}

			return false;
		}

		internal static bool ShouldSerializeProperty(PropertyInfo property)
		{
			// First check for reasons to NOT serialize

			if (property.GetGetMethod().IsStatic || property.GetMethod is null || property.SetMethod is null || property.GetMethod.GetParameters().Length != 0)
			{
				return false;
			}

			// Prohibiting attribute is stronger
			foreach (var attr in property.CustomAttributes)
			{

				if (attr.AttributeType == typeof(DoNotExposeAttribute))
				{
					return false;
				}
			}

			if (property.GetMethod.IsPublic && property.SetMethod.IsPublic)
			{
				return true;
			}

			foreach (var attr in property.CustomAttributes)
			{
				if (attr.AttributeType == typeof(ExposeAttribute))
				{
					return true;
				}
			}

			return false;
		}


		internal static Array GetEnumValues(Type enumType)
		{
			return Enum.GetValues(enumType);
		}


		internal static bool IsPrimitive(Type type)
		{
			return type.IsPrimitive;
		}


		internal static object ParseFieldValue(FieldInfo field, string value)
		{
			s_ObjectArrayTwoElems[0] = value;
			s_ObjectArrayTwoElems[1] = CultureInfo.InvariantCulture;
			s_TypeArrayTwoElems[0] = s_stringType;
			s_TypeArrayTwoElems[1] = s_cultureInfoType;

			var parseMethod = field.FieldType.GetMethod("Parse", s_TypeArrayTwoElems);
			var parsedValue = parseMethod.Invoke(null, s_ObjectArrayTwoElems);
			return parsedValue;
		}


		internal static object ParsePropertyValue(PropertyInfo property, string value)
		{
			s_ObjectArrayTwoElems[0] = value;
			s_ObjectArrayTwoElems[1] = CultureInfo.InvariantCulture;
			s_TypeArrayTwoElems[0] = s_stringType;
			s_TypeArrayTwoElems[1] = s_cultureInfoType;

			var parseMethod = property.GetMethod.ReturnType.GetMethod("Parse", s_TypeArrayTwoElems);
			var parsedValue = parseMethod.Invoke(null, s_ObjectArrayTwoElems);
			return parsedValue;
		}


		internal static object ParseEnumValue(Type enumType, string value)
		{
			return Enum.Parse(enumType, value);
		}


		internal static object EnumToUnderlyingType(Type enumType, object enumValue)
		{
			var underlyingType = Enum.GetUnderlyingType(enumType);
			var underlyingValue = Convert.ChangeType(enumValue, underlyingType);
			return underlyingValue;
		}
	}
}
