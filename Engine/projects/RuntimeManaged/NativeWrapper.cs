using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    public abstract class NativeWrapper
    {
        protected ulong _id;
        protected IntPtr _ptr;


        public static void Destroy(NativeWrapper wrapper)
        {
            InternalDestroyMAO(wrapper._id);
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void InternalDestroyMAO(ulong _id);
    }
}
