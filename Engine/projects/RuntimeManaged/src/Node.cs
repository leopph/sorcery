using System.Runtime.InteropServices;


[StructLayout(LayoutKind.Sequential)]
public class Node : leopph.Object
{
    [DllImport("leopph_runtime_native.dll", EntryPoint = "new_node")]
    private extern static ulong NewNode();


    public Node()
    {
        _id = NewNode();
    }

    private ulong _id;
}