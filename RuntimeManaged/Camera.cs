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
		
		public extern CameraType Type
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		
		// Horizontal FOV
		public extern float PerspectiveFieldOfView
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		
		// Horizontal Size
		public extern float OrthographicSize
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		
		public extern float NearClipPlane
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		
		public extern float FarClipPlane
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}
	}
}
