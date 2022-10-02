using System;

namespace leopph
{
    public struct Color
    {
        public Color(byte red = 0, byte green = 0, byte blue = 0, byte alpha = 255)
        {
            R = red;
            G = green;
            B = blue;
            A = alpha;
        }

        public byte R { get; set; }
        public byte G { get; set; }
        public byte B { get; set; }
        public byte A { get; set; }

        public static Color Black => new Color(0, 0, 0, 255);
        public static Color White => new Color(255, 255, 255, 255);
        public static Color Red => new Color(255, 0, 0, 255);
        public static Color Green => new Color(0, 255, 0, 255);
        public static Color Blue => new Color(0, 0, 255, 255);
        public static Color Cyan => new Color(0, 255, 255, 255);
        public static Color Magenta => new Color(255, 0, 255, 255);
        public static Color Yellow => new Color(255, 255, 0, 255);

        public byte this[int index]
        {
            get => index switch
            {
                0 => R,
                1 => G,
                2 => B,
                3 => A,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: R = value; break;
                    case 1: G = value; break;
                    case 2: B = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }
    }
}