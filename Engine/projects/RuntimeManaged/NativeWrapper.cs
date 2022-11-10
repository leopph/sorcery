using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace leopph {
	[StructLayout(LayoutKind.Sequential)]
	public abstract class NativeWrapper {
		private protected ulong _id;
		private protected IntPtr _ptr;

		public extern Guid Guid {
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}
	}
}
