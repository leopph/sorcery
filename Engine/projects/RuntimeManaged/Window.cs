using System.Runtime.CompilerServices;

namespace leopph
{
    public static class Window
    {
        public static Extent2D CurrentResolution => InternalGetCurrentClientAreaSize();


        public static Extent2D WindowedResolution
        {
            get => InternalGetWindowedClientAreaSize();
            set => InternalSetWindowedClientAreaSize(value);
        }


        public static bool IsBorderless
        {
            get => InternalIsBorderless() != 0;
            set => InternalSetBorderless(value ? 1 : 0);
        }


        public static bool IsMinimizingOnBorderlessFocusLoss
        {
            get => InternalIsMinimizingOnBorderlessFocusLoss() != 0;
            set => InternalSetMinimizeOnBorderlessFocusLoss(value ? 1 : 0);
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Extent2D InternalGetCurrentClientAreaSize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Extent2D InternalGetWindowedClientAreaSize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void InternalSetWindowedClientAreaSize(Extent2D clientAreaSize);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int InternalIsBorderless();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void InternalSetBorderless(int borderless);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int InternalIsMinimizingOnBorderlessFocusLoss();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void InternalSetMinimizeOnBorderlessFocusLoss(int minimize);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void InternalSetShouldClose(bool shouldClose);
    }
}