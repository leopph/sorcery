using System;


namespace leopph
{
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
	public class ExposeAttribute : Attribute { }

	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
	public class DoNotExposeAttribute : Attribute { }
}
