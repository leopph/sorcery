using System.Runtime.CompilerServices;

namespace leopph
{
    public class Component : NativeWrapper
    {
        private protected Component() { }

		[Expose]
        public extern Entity Entity
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}

		[Expose]
		public extern Transform Transform
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}
    }
}
