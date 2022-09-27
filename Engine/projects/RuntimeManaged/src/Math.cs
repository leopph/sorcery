namespace leopph
{
    public static class Math
    {
        public static float Pi => (float)System.Math.PI;

        public static float ToDegrees(float radians)
        {
            return radians * 180.0f / Pi;
        }

        public static float ToRadians(float degrees)
        {
            return degrees * Pi / 180.0f;
        }

        public static float Sqrt(float number)
        {
            return (float)System.Math.Sqrt(number);
        }

        public static float Pow(float baseNum, int exp)
        {
            return (float)System.Math.Pow(baseNum, exp);
        }

        public static float Sin(float number)
        {
            return (float)System.Math.Sin(number);
        }

        public static float Cos(float number)
        {
            return (float)System.Math.Cos(number);
        }
    }


    public struct Vector2
    {
        private readonly float[] _data;

        public Vector2(float x = 0, float y = 0)
        {
            _data = new float[2];
            _data[0] = x;
            _data[1] = y;
        }

        public static Vector2 Right => new Vector2(1, 0);
        public static Vector2 Left => new Vector2(-1, 0);
        public static Vector2 Up => new Vector2(0, 1);
        public static Vector2 Down => new Vector2(0, -1);
        public static Vector2 Zero => new Vector2(0, 0);
        public static Vector2 One => new Vector2(1, 1);

        public float X
        {
            get { return _data[0]; }
            set { _data[0] = value; }
        }

        public float Y
        {
            get { return _data[1]; }
            set { _data[1] = value; }
        }

        public float this[int index]
        {
            get { return _data[index]; }
            set { _data[index] = value; }
        }

        public static Vector2 operator -(Vector2 vec) => new Vector2(-vec.X, -vec.Y);

        public static Vector2 operator +(Vector2 left, Vector2 right) => new Vector2(left.X + right.X, left.Y + right.Y);
        public static Vector2 operator -(Vector2 left, Vector2 right) => new Vector2(left.X - right.X, left.Y - right.Y);

        public static Vector2 operator *(Vector2 left, Vector2 right) => new Vector2(left.X * right.X, left.Y * right.Y);
        public static Vector2 operator *(Vector2 left, float right) => new Vector2(left.X * right, left.Y * right);
        public static Vector2 operator *(float left, Vector2 right) => new Vector2(left * right.X, left * right.Y);

        public static Vector2 operator /(Vector2 left, Vector2 right) => new Vector2(left.X / right.X, left.Y / right.Y);
        public static Vector2 operator /(Vector2 left, float right) => new Vector2(left.X / right, left.Y / right);

        public static bool operator ==(Vector2 left, Vector2 right) => left.X == right.X && left.Y == right.Y;
        public static bool operator !=(Vector2 left, Vector2 right) => !(left == right);

        public override bool Equals(object other) => other is Vector2 && this == (Vector2)other;

        public static explicit operator Vector3(Vector2 vec) => new Vector3(vec.X, vec.Y, 0);
        public static explicit operator Vector4(Vector2 vec) => new Vector4(vec.X, vec.Y, 0, 1);

        public float Length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2));

        public Vector2 Normalize()
        {
            var lngth = Length;
            X /= lngth;
            Y /= lngth;
            return this;
        }

        public Vector2 Normalized => new Vector2(X, Y).Normalize();
    }


    public class Vector3
    {
        private readonly float[] _data = new float[3];

        public Vector3(float x = 0, float y = 0, float z = 0)
        {
            _data[0] = x;
            _data[1] = y;
            _data[2] = z;
        }

        public static Vector3 Right => new Vector3(1, 0, 0);
        public static Vector3 Left => new Vector3(-1, 0, 0);
        public static Vector3 Up => new Vector3(0, 1, 0);
        public static Vector3 Down => new Vector3(0, -1, 0);
        public static Vector3 Forward => new Vector3(0, 0, 1);
        public static Vector3 Back => new Vector3(0, 0, -1);
        public static Vector3 Zero => new Vector3(0, 0, 0);
        public static Vector3 One => new Vector3(1, 1, 1);

        public float X
        {
            get { return _data[0]; }
            set { _data[0] = value; }
        }

        public float Y
        {
            get { return _data[1]; }
            set { _data[1] = value; }
        }

        public float Z
        {
            get { return _data[2]; }
            set { _data[2] = value; }
        }

        public float this[int index]
        {
            get { return _data[index]; }
            set { _data[index] = value; }
        }

        public static Vector3 operator -(Vector3 vec) => new Vector3(-vec.X, -vec.Y, -vec.Z);

        public static Vector3 operator +(Vector3 left, Vector3 right) => new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
        public static Vector3 operator -(Vector3 left, Vector3 right) => new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);

        public static Vector3 operator *(Vector3 left, Vector3 right) => new Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
        public static Vector3 operator *(Vector3 left, float right) => new Vector3(left.X * right, left.Y * right, left.Z * right);
        public static Vector3 operator *(float left, Vector3 right) => new Vector3(left * right.X, left * right.Y, left * right.Z);

        public static Vector3 operator /(Vector3 left, Vector3 right) => new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
        public static Vector3 operator /(Vector3 left, float right) => new Vector3(left.X / right, left.Y / right, left.Z / right);

        public static bool operator ==(Vector3 left, Vector3 right) => left.X == right.X && left.Y == right.Y && left.Z == right.Z;
        public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

        public override bool Equals(object other) => other is Vector3 && this == (Vector3)other;

        public static explicit operator Vector2(Vector3 vec) => new Vector2(vec.X, vec.Y);
        public static explicit operator Vector4(Vector3 vec) => new Vector4(vec.X, vec.Y, 0, 1);

        public float length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2));

        public Vector3 Normalize()
        {
            var lngth = length;
            X /= lngth;
            Y /= lngth;
            Z /= lngth;
            return this;
        }

        public Vector3 Normalized => new Vector3(X, Y, Z).Normalize();
    }


    public class Vector4
    {
        private readonly float[] _data = new float[4];

        public Vector4(float x = 0, float y = 0, float z = 0, float w = 1)
        {
            _data[0] = x;
            _data[1] = y;
            _data[2] = z;
            _data[3] = w;
        }

        public float X
        {
            get { return _data[0]; }
            set { _data[0] = value; }
        }

        public float Y
        {
            get { return _data[1]; }
            set { _data[1] = value; }
        }

        public float Z
        {
            get { return _data[2]; }
            set { _data[2] = value; }
        }

        public float W
        {
            get { return _data[3]; }
            set { _data[3] = value; }
        }

        public float this[int index]
        {
            get { return _data[index]; }
            set { _data[index] = value; }
        }

        public static Vector4 operator -(Vector4 vec) => new Vector4(-vec.X, -vec.Y, -vec.Z, -vec.W);

        public static Vector4 operator +(Vector4 left, Vector4 right) => new Vector4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);
        public static Vector4 operator -(Vector4 left, Vector4 right) => new Vector4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);

        public static Vector4 operator *(Vector4 left, Vector4 right) => new Vector4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);
        public static Vector4 operator *(Vector4 left, float right) => new Vector4(left.X * right, left.Y * right, left.Z * right, left.W * right);
        public static Vector4 operator *(float left, Vector4 right) => new Vector4(left * right.X, left * right.Y, left * right.Z, left * right.W);

        public static Vector4 operator /(Vector4 left, Vector4 right) => new Vector4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);
        public static Vector4 operator /(Vector4 left, float right) => new Vector4(left.X / right, left.Y / right, left.Z / right, left.W / right);

        public static bool operator ==(Vector4 left, Vector4 right) => left.X == right.X && left.Y == right.Y && left.Z == right.Z && left.W == right.W;
        public static bool operator !=(Vector4 left, Vector4 right) => !(left == right);

        public override bool Equals(object other) => other is Vector4 && this == (Vector4)other;

        public static explicit operator Vector2(Vector4 vec) => new Vector2(vec.X, vec.Y);
        public static explicit operator Vector3(Vector4 vec) => new Vector3(vec.X, vec.Y, vec.Z);

        public float length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2) + Math.Pow(W, 2));

        public Vector4 Normalize()
        {
            var lngth = length;
            X /= lngth;
            Y /= lngth;
            Z /= lngth;
            W /= lngth;
            return this;
        }

        public Vector4 Normalized => new Vector4(X, Y, Z, W).Normalize();
    }


    public struct Matrix3
    {
        public Matrix3(float e00 = 1, float e01 = 0, float e02 = 0, float e10 = 0, float e11 = 1, float e12 = 0, float e20 = 0, float e21 = 0, float e22 = 1)
        {
            _data = new Vector3[3];
            _data[0][0] = e00;
            _data[0][1] = e01;
            _data[0][2] = e02;
            _data[1][0] = e10;
            _data[1][1] = e11;
            _data[1][2] = e12;
            _data[2][0] = e20;
            _data[2][1] = e21;
            _data[2][2] = e22;
        }

        public static Matrix3 Identity => new Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);

        public float this[int row, int column]
        {
            get { return _data[row][column]; }
            set { _data[row][column] = value; }
        }

        private readonly Vector3[] _data;
    }


    public struct Matrix4
    {
        public Matrix4(float e00 = 1, float e01 = 0, float e02 = 0, float e03 = 0, float e10 = 0, float e11 = 1, float e12 = 0, float e13 = 0, float e20 = 0, float e21 = 0, float e22 = 1, float e23 = 0, float e30 = 0, float e31 = 0, float e32 = 0, float e33 = 1)
        {
            _data = new Vector4[4];
            _data[0][0] = e00;
            _data[0][1] = e01;
            _data[0][2] = e02;
            _data[0][3] = e03;
            _data[1][0] = e10;
            _data[1][1] = e11;
            _data[1][2] = e12;
            _data[1][3] = e13;
            _data[2][0] = e20;
            _data[2][1] = e21;
            _data[2][2] = e22;
            _data[2][3] = e23;
            _data[3][0] = e30;
            _data[3][1] = e31;
            _data[3][2] = e32;
            _data[3][3] = e33;
        }

        public static Matrix4 Identity => new Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

        public float this[int row, int column]
        {
            get { return _data[row][column]; }
            set { _data[row][column] = value; }
        }

        private readonly Vector4[] _data;
    }


    public struct Quaternion
    {
        private readonly float[] _data;

        public Quaternion(float w = 1, float x = 0, float y = 0, float z = 0)
        {
            _data = new float[4];
            _data[0] = w;
            _data[1] = x;
            _data[2] = y;
            _data[3] = z;
        }

        public Quaternion(Vector3 axis, float angleDegrees)
        {
            var angleHalfRadians = Math.ToRadians(angleDegrees) / 2.0f;
            var vec = axis.Normalized * Math.Sin(angleHalfRadians);

            _data = new float[4]
            {
                Math.Cos(angleHalfRadians),
                vec[0],
                vec[1],
                vec[2],
            };
        }

        public static Quaternion Identity => new Quaternion(1, 0, 0, 0);

        public float W
        {
            get => _data[0];
            set => _data[0] = value;
        }

        public float X
        {
            get => _data[1];
            set => _data[1] = value;
        }

        public float Y
        {
            get => _data[2];
            set => _data[2] = value;
        }

        public float Z
        {
            get => _data[3];
            set => _data[3] = value;
        }

        public float Norm => Math.Sqrt(Math.Pow(W, 2) + Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 3));

        public Quaternion Normalize()
        {
            var norm = Norm;
            W /= norm;
            X /= norm;
            Y /= norm;
            Z /= norm;
            return this;
        }

        public Quaternion Normalized => new Quaternion(W, X, Y, Z).Normalize();

        public Quaternion Conjugate => new Quaternion(W, -X, -Y, -Z);

        public Quaternion ConjugateInPlace()
        {
            X = -X;
            Y = -Y;
            Z = -Z;
            return this;
        }

        public Quaternion Invert()
        {
            var normSquared = Math.Pow(W, 2) + Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2);
            W /= normSquared;
            X /= normSquared;
            Y /= normSquared;
            Z /= normSquared;
            return this;
        }

        public Quaternion Inverse => new Quaternion(W, X, Y, Z).Invert();

        public static Quaternion operator *(Quaternion left, Quaternion right)
        {
            var w = left.W * right.W - left.X * right.X - left.Y * right.Y - left.Z * right.Z;
            var x = left.W * right.X + left.X * right.W + left.Y * right.Z - left.Z * right.Y;
            var y = left.W * right.Y - left.X * right.Z + left.Y * right.W + left.Z * right.X;
            var z = left.W * right.Z + left.X * right.Y - left.Y * right.X + left.Z * right.W;
            return new Quaternion(w, x, y, z);
        }

        public float this[int index]
        {
            get { return _data[index]; }
            set { _data[index] = value; }
        }
    }
}