using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System;

namespace leopph
{
    public class Component : NativeWrapper
    {
        private protected Component() { }

        public Entity Entity => (GCHandle.FromIntPtr(InternalGetEntityHandle(_ptr)).Target as Entity)!;


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static IntPtr InternalGetEntityHandle(IntPtr ptr);
    }
}
