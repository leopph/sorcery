using System.Runtime.InteropServices;

namespace leopph
{
    public static class Time
    {
        [DllImport("leopph_runtime_native.dll", EntryPoint = "get_frame_time")]
        private extern static float GetFrameTime();

        public static float FrameTime => GetFrameTime();
    }
}
