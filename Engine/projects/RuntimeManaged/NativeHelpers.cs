using System;
using System.Reflection;

namespace leopph
{
	internal static class NativeHelpers
	{
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


		internal static Array GetEnumValues(string enumTypeName)
		{
			var enumType = Type.GetType(enumTypeName);
			Console.WriteLine(enumType.FullName);
			return Enum.GetValues(enumType);
		}


		internal static bool IsPrimitiveOrString(Type type)
		{
			return type.IsPrimitive || type == typeof(string);
		}
	}
}
