using System.Runtime.InteropServices;

namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    public abstract class NativeWrapper
    {
        protected readonly ulong _id;

        protected NativeWrapper(ulong id = 0) => _id = id;
    }
}
