using System.Runtime.InteropServices;

class Test
{
    [DllImport("RuntimeUnmanaged.dll", EntryPoint = "add_position")]
    private extern static void AddPosition(float[] vec);

    public static void DoTest()
    {
        AddPosition(new float[3] { -1, 0, 3 });
        AddPosition(new float[3] { 1, 0, 3 });
        AddPosition(new float[3] { 0, 0, 6 });
    }
}