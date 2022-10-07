#pragma warning disable CS0659
#pragma warning disable CS0661

using System;

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

        public static float Abs(float number)
        {
            return (float)System.Math.Abs(number);
        }

        public static bool NearlyEqual(float left, float right)
        {
            return Abs(left - right) < float.Epsilon;
        }
    }


    public struct Vector2
    {
        public Vector2(float x = 0, float y = 0) => (X, Y) = (x, y);

        public float X { get; set; }
        public float Y { get; set; }

        public static Vector2 Right => new Vector2(1, 0);
        public static Vector2 Left => new Vector2(-1, 0);
        public static Vector2 Up => new Vector2(0, 1);
        public static Vector2 Down => new Vector2(0, -1);
        public static Vector2 Zero => new Vector2(0, 0);
        public static Vector2 One => new Vector2(1, 1);

        public float this[int index]
        {
            get => index switch
            {
                0 => X,
                1 => Y,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: X = value; break;
                    case 1: Y = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Vector2 operator -(Vector2 vec) => new Vector2(-vec.X, -vec.Y);

        public static Vector2 operator +(Vector2 left, Vector2 right) => new Vector2(left.X + right.X, left.Y + right.Y);
        public static Vector2 operator -(Vector2 left, Vector2 right) => new Vector2(left.X - right.X, left.Y - right.Y);

        public static Vector2 operator *(Vector2 left, Vector2 right) => new Vector2(left.X * right.X, left.Y * right.Y);
        public static Vector2 operator *(Vector2 left, float right) => new Vector2(left.X * right, left.Y * right);
        public static Vector2 operator *(float left, Vector2 right) => new Vector2(left * right.X, left * right.Y);

        public static Vector2 operator /(Vector2 left, Vector2 right) => new Vector2(left.X / right.X, left.Y / right.Y);
        public static Vector2 operator /(Vector2 left, float right) => new Vector2(left.X / right, left.Y / right);

        public static bool operator ==(Vector2 left, Vector2 right) => Math.NearlyEqual(left.X, right.X) && Math.NearlyEqual(left.Y, right.Y);
        public static bool operator !=(Vector2 left, Vector2 right) => !(left == right);

        public override bool Equals(object other) => other is Vector2 vector && X == vector.X && Y == vector.Y;

        public static explicit operator Vector3(Vector2 vec) => new Vector3(vec.X, vec.Y, 0);
        public static explicit operator Vector4(Vector2 vec) => new Vector4(vec.X, vec.Y, 0, 1);

        public float Length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2));

        public Vector2 Normalize()
        {
            var lngth = Length;

            if (Math.NearlyEqual(lngth, 0))
            {
                X = 0;
                Y = 0;
            }
            else
            {
                X /= lngth;
                Y /= lngth;
            }

            return this;
        }

        public Vector2 Normalized => new Vector2(X, Y).Normalize();

        public override string ToString() => $"({X}, {Y})";
    }


    public struct Vector3
    {
        public Vector3(float x = 0, float y = 0, float z = 0) => (X, Y, Z) = (x, y, z);

        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }

        public static Vector3 Right => new Vector3(1, 0, 0);
        public static Vector3 Left => new Vector3(-1, 0, 0);
        public static Vector3 Up => new Vector3(0, 1, 0);
        public static Vector3 Down => new Vector3(0, -1, 0);
        public static Vector3 Forward => new Vector3(0, 0, 1);
        public static Vector3 Backward => new Vector3(0, 0, -1);
        public static Vector3 Zero => new Vector3(0, 0, 0);
        public static Vector3 One => new Vector3(1, 1, 1);

        public float this[int index]
        {
            get => index switch
            {
                0 => X,
                1 => Y,
                2 => Z,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: X = value; break;
                    case 1: Y = value; break;
                    case 2: Z = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Vector3 operator -(Vector3 vec) => new Vector3(-vec.X, -vec.Y, -vec.Z);

        public static Vector3 operator +(Vector3 left, Vector3 right) => new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
        public static Vector3 operator -(Vector3 left, Vector3 right) => new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);

        public static Vector3 operator *(Vector3 left, Vector3 right) => new Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
        public static Vector3 operator *(Vector3 left, float right) => new Vector3(left.X * right, left.Y * right, left.Z * right);
        public static Vector3 operator *(float left, Vector3 right) => new Vector3(left * right.X, left * right.Y, left * right.Z);

        public static Vector3 operator /(Vector3 left, Vector3 right) => new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
        public static Vector3 operator /(Vector3 left, float right) => new Vector3(left.X / right, left.Y / right, left.Z / right);

        public static bool operator ==(Vector3 left, Vector3 right) => Math.NearlyEqual(left.X, right.X) && Math.NearlyEqual(left.Y, right.Y) && Math.NearlyEqual(left.Z, right.Z);
        public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

        public override bool Equals(object other) => other is Vector3 vector && X == vector.X && Y == vector.Y && Z == vector.Z;

        public static explicit operator Vector2(Vector3 vec) => new Vector2(vec.X, vec.Y);
        public static explicit operator Vector4(Vector3 vec) => new Vector4(vec.X, vec.Y, 0, 1);

        public float Length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2));

        public Vector3 Normalize()
        {
            var lngth = Length;

            if (Math.NearlyEqual(lngth, 0))
            {
                X = 0;
                Y = 0;
                Z = 0;
            }
            else
            {
                X /= lngth;
                Y /= lngth;
                Z /= lngth;
            }

            return this;
        }

        public Vector3 Normalized => new Vector3(X, Y, Z).Normalize();

        public override string ToString() => $"({X}, {Y}, {Z})";
    }


    public struct Vector4
    {
        public Vector4(float x = 0, float y = 0, float z = 0, float w = 1) => (X, Y, Z, W) = (x, y, z, w);

        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }
        public float W { get; set; }

        public float this[int index]
        {
            get => index switch
            {
                0 => X,
                1 => Y,
                2 => Z,
                3 => W,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: X = value; break;
                    case 1: Y = value; break;
                    case 2: Z = value; break;
                    case 3: W = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Vector4 operator -(Vector4 vec) => new Vector4(-vec.X, -vec.Y, -vec.Z, -vec.W);

        public static Vector4 operator +(Vector4 left, Vector4 right) => new Vector4(left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W);
        public static Vector4 operator -(Vector4 left, Vector4 right) => new Vector4(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);

        public static Vector4 operator *(Vector4 left, Vector4 right) => new Vector4(left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W);
        public static Vector4 operator *(Vector4 left, float right) => new Vector4(left.X * right, left.Y * right, left.Z * right, left.W * right);
        public static Vector4 operator *(float left, Vector4 right) => new Vector4(left * right.X, left * right.Y, left * right.Z, left * right.W);

        public static Vector4 operator /(Vector4 left, Vector4 right) => new Vector4(left.X / right.X, left.Y / right.Y, left.Z / right.Z, left.W / right.W);
        public static Vector4 operator /(Vector4 left, float right) => new Vector4(left.X / right, left.Y / right, left.Z / right, left.W / right);

        public static bool operator ==(Vector4 left, Vector4 right) => Math.NearlyEqual(left.X, right.X) && Math.NearlyEqual(left.Y, right.Y) && Math.NearlyEqual(left.Z, right.Z) && Math.NearlyEqual(left.W, right.W);
        public static bool operator !=(Vector4 left, Vector4 right) => !(left == right);

        public override bool Equals(object other) => other is Vector4 vector && X == vector.X && Y == vector.Y && Z == vector.Z && W == vector.W;

        public static explicit operator Vector2(Vector4 vec) => new Vector2(vec.X, vec.Y);
        public static explicit operator Vector3(Vector4 vec) => new Vector3(vec.X, vec.Y, vec.Z);

        public float Length => Math.Sqrt(Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2) + Math.Pow(W, 2));

        public Vector4 Normalize()
        {
            var lngth = Length;

            if (Math.NearlyEqual(lngth, 0))
            {
                X = 0;
                Y = 0;
                Z = 0;
                W = 0;
            }
            else
            {
                X /= lngth;
                Y /= lngth;
                Z /= lngth;
                W /= lngth;
            }

            return this;
        }

        public Vector4 Normalized => new Vector4(X, Y, Z, W).Normalize();

        public override string ToString() => $"({X}, {Y}, {Z}, {W})";
    }


    public struct Matrix3
    {
        public Matrix3(float e00 = 1, float e01 = 0, float e02 = 0,
                        float e10 = 0, float e11 = 1, float e12 = 0,
                        float e20 = 0, float e21 = 0, float e22 = 1) =>
                            (_0, _1, _2) = (new Vector3(e00, e01, e02),
                                            new Vector3(e10, e11, e12),
                                            new Vector3(e20, e21, e22));

        public float this[int row, int column]
        {
            get => row switch
            {
                0 => _0[column],
                1 => _1[column],
                2 => _2[column],
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (row)
                {
                    case 0: _0[column] = value; break;
                    case 1: _1[column] = value; break;
                    case 2: _2[column] = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Matrix3 Identity => new Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);

        private Vector3 _0, _1, _2;
    }


    public struct Matrix4
    {
        public Matrix4(float e00 = 1, float e01 = 0, float e02 = 0, float e03 = 0,
                        float e10 = 0, float e11 = 1, float e12 = 0, float e13 = 0,
                        float e20 = 0, float e21 = 0, float e22 = 1, float e23 = 0,
                        float e30 = 0, float e31 = 0, float e32 = 0, float e33 = 1) =>
                            (_0, _1, _2, _3) = (new Vector4(e00, e01, e02, e03),
                                                new Vector4(e10, e11, e12, e13),
                                                new Vector4(e20, e21, e22, e23),
                                                new Vector4(e30, e31, e32, e33));

        public float this[int row, int column]
        {
            get => row switch
            {
                0 => _0[column],
                1 => _1[column],
                2 => _2[column],
                3 => _3[column],
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (row)
                {
                    case 0: _0[column] = value; break;
                    case 1: _1[column] = value; break;
                    case 2: _2[column] = value; break;
                    case 3: _3[column] = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Matrix4 Identity => new Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

        private Vector4 _0, _1, _2, _3;
    }


    public struct Quaternion
    {
        public Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) => (X, Y, Z, W) = (x, y, z, w);

        public Quaternion(Vector3 axis, float angleDegrees)
        {
            var angleHalfRadians = Math.ToRadians(angleDegrees) / 2.0f;
            var vec = axis.Normalized * Math.Sin(angleHalfRadians);

            X = vec.X;
            Y = vec.Y;
            Z = vec.Z;
            W = Math.Cos(angleHalfRadians);
        }

        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }
        public float W { get; set; }

        public float this[int index]
        {
            get => index switch
            {
                0 => X,
                1 => Y,
                2 => Z,
                3 => W,
                _ => throw new IndexOutOfRangeException()
            };
            set
            {
                switch (index)
                {
                    case 0: X = value; break;
                    case 1: Y = value; break;
                    case 2: Z = value; break;
                    case 3: W = value; break;
                    default: throw new IndexOutOfRangeException();
                }
            }
        }

        public static Quaternion Identity => new Quaternion(0, 0, 0, 1);

        public float Norm => Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 3) + Math.Sqrt(Math.Pow(W, 2));

        public Quaternion Normalize()
        {
            var norm = Norm;
            X /= norm;
            Y /= norm;
            Z /= norm;
            W /= norm;
            return this;
        }

        public Quaternion Normalized => new Quaternion(X, Y, Z, W).Normalize();

        public Quaternion Conjugate => new Quaternion(-X, -Y, -Z, W);

        public Quaternion ConjugateInPlace()
        {
            X = -X;
            Y = -Y;
            Z = -Z;
            return this;
        }

        public Quaternion Invert()
        {
            var normSquared = Math.Pow(X, 2) + Math.Pow(Y, 2) + Math.Pow(Z, 2) + Math.Pow(W, 2);
            X = -X / normSquared;
            Y = -Y / normSquared;
            Z = -Z / normSquared;
            W /= normSquared;
            return this;
        }

        public Quaternion Inverse => new Quaternion(X, Y, Z, W).Invert();

        public static Quaternion operator *(Quaternion left, Quaternion right)
        {
            var x = left.W * right.X + left.X * right.W + left.Y * right.Z - left.Z * right.Y;
            var y = left.W * right.Y - left.X * right.Z + left.Y * right.W + left.Z * right.X;
            var z = left.W * right.Z + left.X * right.Y - left.Y * right.X + left.Z * right.W;
            var w = left.W * right.W - left.X * right.X - left.Y * right.Y - left.Z * right.Z;
            return new Quaternion(x, y, z, w);
        }
    }
}
