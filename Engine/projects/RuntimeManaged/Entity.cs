#pragma warning disable CS0659
#pragma warning disable CS0661

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


namespace leopph
{
    public enum Space : byte
    {
        World = 0,
        Object = 1
    }


    [StructLayout(LayoutKind.Sequential)]
    public class Entity
    {
        private readonly ulong _id;


        public Entity() : this(NativeNewEntity())
        {}


        private Entity(ulong id)
        {
            _id = id;
        }


        public void Destroy()
        {
            NativeDeleteEntity(_id);
        }


        public Vector3 Position
        {
            get => NativeGetWorldPos(_id);
            set => NativeSetWorldPos(_id, in value);
        }


        public Vector3 LocalPosition
        {
            get => NativeGetLocalPos(_id);
            set => NativeSetLocalPos(_id, in value);
        }


        public Quaternion Rotation
        {
            get => NativeGetWorldRot(_id);
            set => NativeSetWorldRot(_id, in value);
        }


        public Quaternion LocalRotation
        {
            get => NativeGetLocalRot(_id);
            set => NativeSetLocalRot(_id, in value);
        }


        public Vector3 Scale
        {
            get => NativeGetWorldScale(_id);
            set => NativeSetWorldScale(_id, in value);
        }


        public Vector3 LocalScale
        {
            get => NativeGetLocalScale(_id);
            set => NativeSetLocalScale(_id, in value);
        }


        public Vector3 Right => NativeGetRightAxis(_id);
        public Vector3 Up => NativeGetUpAxis(_id);
        public Vector3 Forward => NativeGetForwardAxis(_id);


        public Entity? Parent
        {
            get
            {
                ulong parentId = NativeGetParentId(_id);
                return parentId == 0 ? null : new Entity(parentId);
            }

            set => NativeSetParent(_id, value == null ? 0 : value._id);
        }


        public ulong ChildCount => NativeGetChildCount(_id);


        public Entity GetChild(ulong index)
        {
            return new Entity(NativeGetChildId(_id, index));
        }


        public void Translate(in Vector3 translation, Space space = Space.World)
        {
            NativeTranslateVector(_id, in translation, space);
        }


        public void Translate(float x, float y, float z, Space space = Space.World)
        {
            NativeTranslate(_id, x, y, z, space);
        }


        public void Rotate(in Quaternion rotation, Space space = Space.World)
        {
            NativeRotate(_id, in rotation, space);
        }


        public void Rotate(in Vector3 axis, float angleDegrees, Space space = Space.World)
        {
            NativeRotateAngleAxis(_id, in axis, angleDegrees, space);
        }


        public void Rescale(in Vector3 scaling, Space space = Space.World)
        {
            NativeRescaleVector(_id, in scaling, space);
        }


        public void Rescale(float x, float y, float z, Space space = Space.World)
        {
            NativeRescale(_id, x, y, z, space);
        }


        public static bool operator ==(Entity? left, Entity? right)
        {
            if (left is null && right is null)
            {
                return true;
            }

            if (left is null && !(right is null))
            {
                return right._id == 0 || NativeIsEntityAlive(right._id) == 0;
            }

            if (!(left is null) && right is null)
            {
                return left._id == 0 || NativeIsEntityAlive(left._id) == 0;
            }

            return left?._id == right?._id;
        }


        public static bool operator !=(Entity? left, Entity? right)
        {
            return !(left == right);
        }


        public override bool Equals(object? obj)
        {
            if (obj == null)
            {
                return NativeIsEntityAlive(_id) == 0;
            }

            if (obj is Entity entity)
            {
                return _id == entity._id;
            }

            return false;
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NativeNewEntity();


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int NativeIsEntityAlive(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeDeleteEntity(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetWorldPos(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldPos(ulong id, in Vector3 position);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetLocalPos(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalPos(ulong id, in Vector3 position);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Quaternion NativeGetWorldRot(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldRot(ulong id, in Quaternion rotation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Quaternion NativeGetLocalRot(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalRot(ulong id, in Quaternion rotation);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetWorldScale(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetWorldScale(ulong id, in Vector3 scale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetLocalScale(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetLocalScale(ulong id, in Vector3 scale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeTranslateVector(ulong id, in Vector3 position, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeTranslate(ulong id, float x, float y, float z, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRotate(ulong id, in Quaternion rotation, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRotateAngleAxis(ulong id, in Vector3 axis, float angleDegrees, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRescaleVector(ulong id, in Vector3 scaling, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeRescale(ulong id, float x, float y, float z, Space space);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetRightAxis(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetUpAxis(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref Vector3 NativeGetForwardAxis(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NativeGetParentId(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void NativeSetParent(ulong targetEntityId, ulong parentEntityId);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NativeGetChildCount(ulong id);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ulong NativeGetChildId(ulong parentId, ulong childIndex);
    }
}