using System;
using System.Runtime.InteropServices;

namespace leopph
{
    public class Object
    {
        [DllImport("leopph_runtime_native.dll", EntryPoint = "delete_node")]
        private extern static void DeleteNode(Node node);


        public static void Destroy(Object obj)
        {
            if (obj is Node node)
            {
                DeleteNode(node);
            }
        }
    }
}
