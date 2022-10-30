using System.Runtime.CompilerServices;

namespace leopph
{
    public class Component : NativeWrapper
    {
        private protected Component() { }

        public extern Entity Entity
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}

		public extern Transform Transform
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}
    }
}
