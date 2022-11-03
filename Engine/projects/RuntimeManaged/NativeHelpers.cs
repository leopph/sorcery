using System;
using System.Reflection;

namespace leopph
{
	internal static class NativeHelpers
	{
		internal static bool IsFieldExposed(FieldInfo field)
		{
			if (field.IsStatic)
			{
				return false;
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

		internal static bool IsPropertyExposed(PropertyInfo property)
		{
			if (property.GetGetMethod().IsStatic)
			{
				return false;
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
	}
}
