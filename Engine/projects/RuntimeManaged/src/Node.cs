#pragma warning disable CS0659
#pragma warning disable CS0661

using System.Runtime.InteropServices;


namespace leopph
{
    public enum Space : byte
    {
        World = 0,
        Object = 1
    }


    [StructLayout(LayoutKind.Sequential)]
    public class Node
    {
        private readonly ulong _id;


        public Node() : this(NativeNewNode())
        {}

        private Node(ulong id)
        {
            _id = id;
        }


        public void Destroy()
        {
            NativeDeleteNode(_id);
        }


        public Vector3 Position
        {
            get => NativeGetNodeWorldPosition(_id);
            set => NativeSetNodeWorldPosition(_id, in value);
        }


        public Vector3 LocalPosition
        {
            get => NativeGetNodeLocalPosition(_id);
            set => NativeSetNodeLocalPosition(_id, in value);
        }


        public Quaternion Rotation
        {
            get => NativeGetNodeWorldRotation(_id);
            set => NativeSetNodeWorldRotation(_id, in value);
        }


        public Quaternion LocalRotation
        {
            get => NativeGetNodeLocalRotation(_id);
            set => NativeSetNodeLocalRotation(_id, in value);
        }


        public Vector3 Scale
        {
            get => NativeGetNodeWorldScale(_id);
            set => NativeSetNodeWorldScale(_id, in value);
        }


        public Vector3 LocalScale
        {
            get => NativeGetNodeLocalScale(_id);
            set => NativeSetNodeLocalScale(_id, in value);
        }


        public Vector3 Right => NativeGetNodeRightAxis(_id);
        public Vector3 Up => NativeGetNodeUpAxis(_id);
        public Vector3 Forward => NativeGetNodeForwardAxis(_id);

        public Node? Parent
        {
            get
            {
                ulong parentId = NativeGetNodeParentId(_id);
                return parentId == 0 ? null : new Node(parentId);
            }

            set => NativeSetNodeParent(_id, value == null ? 0 : value._id);
        }


        public void Translate(in Vector3 translation, Space space = Space.World)
        {
            NativeTranslateNodeFromVector(_id, in translation, space);
        }


        public void Translate(float x, float y, float z, Space space = Space.World)
        {
            NativeTranslateNode(_id, x, y, z, space);
        }


        public void Rotate(in Quaternion rotation, Space space = Space.World)
        {
            NativeRotateNode(_id, in rotation, space);
        }


        public void Rotate(in Vector3 axis, float angleDegrees, Space space = Space.World)
        {
            NativeRotateNodeAngleAxis(_id, in axis, angleDegrees, space);
        }


        public void Rescale(in Vector3 scaling, Space space = Space.World)
        {
            NativeRescaleNodeFromVector(_id, in scaling, space);
        }


        public void Rescale(float x, float y, float z, Space space = Space.World)
        {
            NativeRescaleNode(_id, x, y, z, space);
        }


        public static bool operator ==(Node? left, Node? right)
        {
            if (left is null && right is null)
            {
                return true;
            }

            if (left is null && !(right is null))
            {
                return right._id == 0 || NativeIsNodeAlive(right._id) == 0;
            }

            if (!(left is null) && right is null)
            {
                return left._id == 0 || NativeIsNodeAlive(left._id) == 0;
            }

            return left?._id == right?._id;
        }


        public static bool operator !=(Node? left, Node? right)
        {
            return !(left == right);
        }


        public override bool Equals(object? obj)
        {
            if (obj == null)
            {
                return NativeIsNodeAlive(_id) == 0;
            }

            if (obj is Node node)
            {
                return _id == node._id;
            }

            return false;
        }

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "new_node")]
        private extern static ulong NativeNewNode();


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "is_node_alive")]


        private extern static int NativeIsNodeAlive(ulong id);
        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "delete_node")]
        private extern static void NativeDeleteNode(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_world_position")]
        private extern static ref Vector3 NativeGetNodeWorldPosition(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_world_position")]
        private extern static void NativeSetNodeWorldPosition(ulong id, in Vector3 position);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_local_position")]
        private extern static ref Vector3 NativeGetNodeLocalPosition(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_local_position")]
        private extern static void NativeSetNodeLocalPosition(ulong id, in Vector3 position);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_world_rotation")]
        private extern static ref Quaternion NativeGetNodeWorldRotation(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_world_rotation")]
        private extern static void NativeSetNodeWorldRotation(ulong id, in Quaternion rotation);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_local_rotation")]
        private extern static ref Quaternion NativeGetNodeLocalRotation(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_local_rotation")]
        private extern static void NativeSetNodeLocalRotation(ulong id, in Quaternion rotation);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_world_scale")]
        private extern static ref Vector3 NativeGetNodeWorldScale(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_world_scale")]
        private extern static void NativeSetNodeWorldScale(ulong id, in Vector3 scale);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_local_scale")]
        private extern static ref Vector3 NativeGetNodeLocalScale(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_local_scale")]
        private extern static void NativeSetNodeLocalScale(ulong id, in Vector3 scale);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "translate_node_from_vector")]
        private extern static void NativeTranslateNodeFromVector(ulong id, in Vector3 position, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "translate_node")]
        private extern static void NativeTranslateNode(ulong id, float x, float y, float z, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "rotate_node")]
        private extern static void NativeRotateNode(ulong id, in Quaternion rotation, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "rotate_node_angle_axis")]
        private extern static void NativeRotateNodeAngleAxis(ulong id, in Vector3 axis, float angleDegrees, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "rescale_node_from_vector")]
        private extern static void NativeRescaleNodeFromVector(ulong id, in Vector3 scaling, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "rescale_node")]
        private extern static void NativeRescaleNode(ulong id, float x, float y, float z, Space space);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_right_axis")]
        private extern static ref Vector3 NativeGetNodeRightAxis(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_up_axis")]
        private extern static ref Vector3 NativeGetNodeUpAxis(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_forward_axis")]
        private extern static ref Vector3 NativeGetNodeForwardAxis(ulong id);


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_parent_id", ExactSpelling = true)]
        private extern static ulong NativeGetNodeParentId(ulong id);

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_parent", ExactSpelling = true)]
        private extern static void NativeSetNodeParent(ulong targetNodeId, ulong parentNodeId);
    }
}