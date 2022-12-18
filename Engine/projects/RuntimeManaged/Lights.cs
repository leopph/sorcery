using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;

namespace leopph {
    public abstract class Light : Component {
        public Vector3 Color {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }

        public float Intensity {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }
    }


    public class DirectionalLight : Light {
        public Vector3 Direction => Entity.Transform.Forward;
    }
}
