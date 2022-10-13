using System.Runtime.CompilerServices;
using System;
using System.Runtime.InteropServices;

namespace leopph
{
    public class Behavior : NativeWrapper
    {
        public Entity Entity => (GCHandle.FromIntPtr(InternalGetEntityHandle(_ptr)).Target as Entity)!;


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static IntPtr InternalGetEntityHandle(IntPtr ptr);
    }
}
