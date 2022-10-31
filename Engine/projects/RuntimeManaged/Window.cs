using System.Runtime.CompilerServices;

namespace leopph
{
    public static class Window
    {
        public extern static Extent2D<uint> CurrentResolution
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}


        public extern static Extent2D<uint> WindowedResolution
        {
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
        }


        public static bool IsBorderless
        {
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
        }


        public static bool IsMinimizingOnBorderlessFocusLoss
        {
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void InternalSetShouldClose(bool shouldClose);
    }
}