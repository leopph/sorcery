#pragma warning disable CS0659
#pragma warning disable CS0661

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace leopph
{
    public enum Space : byte
    {
        World = 0,
        Object = 1
    }


    public class Entity : NativeWrapper
    {
        public Entity()
        {
            NativeNew(this);
        }


        public Vector3 Position
        {
            get => NativeGetWorldPos(_ptr);
            set => NativeSetWorldPos(_ptr, in value);
        }


        public Vector3 LocalPosition
        {
            get => NativeGetLocalPos(_ptr);
            set => NativeSetLocalPos(_ptr, in value);
        }


        public Quaternion Rotation
        {
            get => NativeGetWorldRot(_ptr);
            set => NativeSetWorldRot(_ptr, in value);
        }


        public Quaternion LocalRotation
        {
            get => NativeGetLocalRot(_ptr);
            set => NativeSetLocalRot(_ptr, in value);
        }


        public Vector3 Scale
        {
            get => NativeGetWorldScale(_ptr);
            set => NativeSetWorldScale(_ptr, in value);
        }


        public Vector3 LocalScale
        {
            get => NativeGetLocalScale(_ptr);
            set => NativeSetLocalScale(_ptr, in value);
        }


        public Vector3 Right => NativeGetRightAxis(_ptr);
        public Vector3 Up => NativeGetUpAxis(_ptr);
        public Vector3 Forward => NativeGetForwardAxis(_ptr);


        public Entity? Parent
        {
            get => GCHandle.FromIntPtr(NativeGetParentHandle(_ptr)).Target as Entity ?? null;
            set => NativeSetParent(_ptr, value == null ? IntPtr.Zero : value._ptr);
        }


        public ulong ChildCount => NativeGetChildCount(_ptr);


        public Entity GetChild(ulong index) => (GCHandle.FromIntPtr(NativeGetChildHandle(_ptr, index)).Target as Entity)!;


        public void Translate(in Vector3 translation, Space space = Space.World)
        {
            NativeTranslateVector(_ptr, in translation, space);
        }


        public void Translate(float x, float y, float z, Space space = Space.World)
        {
            NativeTranslate(_ptr, x, y, z, space);
        }


        public void Rotate(in Quaternion rotation, Space space = Space.World)
        {
            NativeRotate(_ptr, in rotation, space);
        }


        public void Rotate(in Vector3 axis, float angleDegrees, Space space = Space.World)
        {
            NativeRotateAngleAxis(_ptr, in axis, angleDegrees, space);
        }


        public void Rescale(in Vector3 scaling, Space space = Space.World)
        {
            NativeRescaleVector(_ptr, in scaling, space);
        }


        public void Rescale(float x, float y, float z, Space space = Space.World)
        {
            NativeRescale(_ptr, x, y, z, space);
        }


        public T CreateBehavior<T>() where T : Behavior => (GCHandle.FromIntPtr(InternalCreateBehavior(typeof(T), _ptr)).Target as T)!;
        

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
        private extern static ulong NativeNew(Entity entity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetWorldPos(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldPos(IntPtr ptr, in Vector3 position);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetLocalPos(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalPos(IntPtr ptr, in Vector3 position);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Quaternion NativeGetWorldRot(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldRot(IntPtr ptr, in Quaternion rotation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Quaternion NativeGetLocalRot(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalRot(IntPtr ptr, in Quaternion rotation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetWorldScale(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldScale(IntPtr ptr, in Vector3 scale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetLocalScale(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalScale(IntPtr ptr, in Vector3 scale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeTranslateVector(IntPtr ptr, in Vector3 position, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeTranslate(IntPtr ptr, float x, float y, float z, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRotate(IntPtr ptr, in Quaternion rotation, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRotateAngleAxis(IntPtr ptr, in Vector3 axis, float angleDegrees, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRescaleVector(IntPtr ptr, in Vector3 scaling, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRescale(IntPtr ptr, float x, float y, float z, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetRightAxis(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetUpAxis(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetForwardAxis(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static IntPtr NativeGetParentHandle(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetParent(IntPtr targetEntityPtr, IntPtr parentEntityPtr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NativeGetChildCount(IntPtr ptr);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static IntPtr NativeGetChildHandle(IntPtr parentPtr, ulong childIndex);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static IntPtr InternalCreateBehavior(Type type, IntPtr entityPtr);
    }
}