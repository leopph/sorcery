using System.Runtime.InteropServices;

namespace leopph
{
    [StructLayout(LayoutKind.Sequential)]
    struct Color
    {
        private byte _red, _green, _blue, _alpha;

        public Color(byte red = 0, byte green = 0, byte blue = 0, byte alpha = 255)
        {
            _red = red;
            _green = green;
            _blue = blue;
            _alpha = alpha;
        }

        public static Color Black => new Color(0, 0, 0, 255);
        public static Color White => new Color(255, 255, 255, 255);
        public static Color Red => new Color(255, 0, 0, 255);
        public static Color Green => new Color(0, 255, 0, 255);
        public static Color Blue => new Color(0, 0, 255, 255);
        public static Color Cyan => new Color(0, 255, 255, 255);
        public static Color Magenta => new Color(255, 0, 255, 255);
        public static Color Yellow => new Color(255, 255, 0, 255);

        public byte R
        {
            get => _red;
            set => _red = value;
        }

        public byte G
        {
            get => _green;
            set => _green = value;
        }

        public byte B
        {
            get => _blue;
            set => _blue = value;
        }

        public byte A
        {
            get => _alpha;
            set => _alpha = value;
        }
    }
}