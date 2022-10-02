using System.Runtime.InteropServices;

namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    public class PointLight
    {
        private Color _color = Color.White;
        private Vector3 _pos = Vector3.Zero;

        public Color Color
        {
            get => _color;
            set => _color = value;
        }

        public Vector3 Position
        {
            get => _pos;
            set => _pos = value;
        }
    }
}