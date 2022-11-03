using System.Runtime.CompilerServices;

namespace leopph
{
	public enum CameraType : byte
	{
		Perspective = 0,
		Orthographic = 1
	}


	public class Camera : Component
	{
		[Expose]
		public extern CameraType Type
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		[Expose]
		// Horizontal FOV
		public extern float PerspectiveFieldOfView
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		[Expose]
		// Horizontal Size
		public extern float OrthographicSize
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		[Expose]
		public extern float NearClipPlane
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		[Expose]
		public extern float FarClipPlane
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}
	}
}
