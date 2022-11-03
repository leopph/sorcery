#pragma warning disable CS0659
#pragma warning disable CS0661

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace leopph
{ 
    public class Entity : NativeWrapper
    {
        public Entity()
        {
			NewNativeEntity(this);
        }


		[Expose]
		public extern string Name
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
			[MethodImpl(MethodImplOptions.InternalCall)]
			set;
		}


		public extern Transform Transform
		{
			[MethodImpl(MethodImplOptions.InternalCall)]
			get;
		}


        public T CreateComponent<T>() where T : Component => (InternalCreateComponent(_ptr, typeof(T)) as T)!;
        

        public static bool operator ==(Entity? left, Entity? right)
        {
            if (left is null && right is null)
            {
                return true;
            }

            if (!(left is null) && right is null)
            {
                return left.Equals(right);
            }

            return right!.Equals(left);
        }


        public static bool operator !=(Entity? left, Entity? right)
        {
            return !(left == right);
        }


        public override bool Equals(object? obj)
        {
            if (obj is null)
            {
                return _ptr == IntPtr.Zero;
            }

            if (obj is Entity entity)
            {
                return _ptr == entity._ptr;
            }

            return false;
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NewNativeEntity(Entity entity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private extern static Component InternalCreateComponent(IntPtr nativeEntityPtr, Type componentType);
    }
}