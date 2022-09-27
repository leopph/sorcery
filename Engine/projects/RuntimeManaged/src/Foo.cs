using System.Runtime.InteropServices;

public class Foo
{
    [DllImport("leopph.dll", EntryPoint = "take_vec2")]
    private extern static void TakeVec2(float[] vec);

    public static void Bar()
    {
        TakeVec2(new float[2] {1, 2 });
    }
}