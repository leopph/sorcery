using System.Runtime.InteropServices;

namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    public class Light
    {
        public Color Color {  get; set; } = Color.White;
    }


    [StructLayout(LayoutKind.Sequential)]
    public class PointLight : Light
    {
        public Vector3 Position { get; set; } = Vector3.One;


        [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_point_light")]
        private extern static void SetLight(PointLight light);


        public static void Test()
        {
            SetLight(new PointLight());
        }
    }
}