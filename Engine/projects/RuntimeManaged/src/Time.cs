using System.Runtime.InteropServices;

namespace leopph
{
    public static class Time
    {
        [DllImport("RuntimeUnmanaged.dll", EntryPoint = "get_frame_time")]
        private extern static float GetFrameTime();

        public static float FrameTime => GetFrameTime();
    }
}
