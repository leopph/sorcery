using System.Runtime.CompilerServices;

namespace leopph
{
	public enum Space : byte
	{
		World = 0,
		Local = 1
	}


	public class Transform : Component
	{
		public extern Vector3 WorldPosition
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}

		[Expose]
		public extern Vector3 LocalPosition
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}

		public extern Quaternion WorldRotation
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}

		[Expose]
		public extern Quaternion LocalRotation
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}

		public extern Vector3 WorldScale
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}

		[Expose]
		public extern Vector3 LocalScale
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		public extern Vector3 Right
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}

		public extern Vector3 Up
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}

		public extern Vector3 Forward
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}


		public extern Transform? Parent
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		public Matrix4 ModelMatrix
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}


		public Matrix3 NormalMatrix
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}


		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Translate(Vector3 translation, Space space = Space.World);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Translate(float x, float y, float z, Space space = Space.World);


		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Rotate(Quaternion rotation, Space space = Space.World);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Rotate(Vector3 axis, float angleDegrees, Space space = Space.World);


		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Rescale(Vector3 scaling, Space space = Space.World);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Rescale(float x, float y, float z, Space space = Space.World);
	}
}
