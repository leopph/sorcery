#pragma warning disable CS0659
#pragma warning disable CS0661

using System.Runtime.InteropServices;


namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    public class Node
    {
        private readonly ulong _id;


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "new_node")]
        private extern static ulong NativeNewNode();

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "delete_node")]
        private extern static void DeleteNode(ulong id);

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "is_node_alive")]
        private extern static int NativeIsNodeAlive(ulong id);

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "get_node_position")]
        private extern static ref Vector3 NativeGetNodePosition(ulong id);

        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_node_position")]
        private extern static void NativeSetNodePosition(ulong id, in Vector3 position);


        public Node()
        {
            _id = NativeNewNode();
        }


        public void Destroy()
        {
            DeleteNode(_id);
        }


        public Vector3 Position
        {
            get => NativeGetNodePosition(_id);
            set => NativeSetNodePosition(_id, in value);
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
    }
}