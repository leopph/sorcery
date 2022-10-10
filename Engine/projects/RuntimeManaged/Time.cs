using System.Runtime.CompilerServices;

namespace leopph
{
    public static class Time
    {
        public static float FullTime
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        public static float FrameTime
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }
    }
}
