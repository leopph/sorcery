namespace leopph
{
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

        public float x
        {
            get { return _data[0]; }
            set { _data[0] = value; }
        }

        public float y
        {
            get { return _data[1]; }
            set { _data[1] = value; }
        }

        public float this[int key]
        {
            get { return _data[key]; }
            set { _data[key] = value; }
        }

        public static Vector2 operator -(Vector2 vec) => new Vector2(-vec.x, -vec.y);

        public static Vector2 operator +(Vector2 left, Vector2 right) => new Vector2(left.x + right.x, left.y + right.y);
        public static Vector2 operator -(Vector2 left, Vector2 right) => new Vector2(left.x - right.x, left.y - right.y);

        public static Vector2 operator *(Vector2 left, Vector2 right) => new Vector2(left.x * right.x, left.y * right.y);
        public static Vector2 operator *(Vector2 left, float right) => new Vector2(left.x * right, left.y * right);
        public static Vector2 operator *(float left, Vector2 right) => new Vector2(left * right.x, left * right.y);

        public static Vector2 operator /(Vector2 left, Vector2 right) => new Vector2(left.x / right.x, left.y / right.y);
        public static Vector2 operator /(Vector2 left, float right) => new Vector2(left.x / right, left.y / right);

        public static bool operator ==(Vector2 left, Vector2 right) => left.x == right.x && left.y == right.y;
        public static bool operator !=(Vector2 left, Vector2 right) => !(left == right);

        public override bool Equals(object other) => other is Vector2 && this == (Vector2)other;

        public static implicit operator Vector3(Vector2 vec) => new Vector3(vec.x, vec.y, 0);
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

        public float x
        {
            get { return _data[0]; }
            set { _data[0] = value; }
        }

        public float y
        {
            get { return _data[1]; }
            set { _data[1] = value; }
        }

        public float z
        {
            get { return _data[2]; }
            set { _data[1] = value; }
        }

        public float this[int key]
        {
            get { return _data[key]; }
            set { _data[key] = value; }
        }

        public static Vector3 operator -(Vector3 vec) => new Vector3(-vec.x, -vec.y, -vec.z);

        public static Vector3 operator +(Vector3 left, Vector3 right) => new Vector3(left.x + right.x, left.y + right.y, left.z + right.z);
        public static Vector3 operator -(Vector3 left, Vector3 right) => new Vector3(left.x - right.x, left.y - right.y, left.z - right.z);

        public static Vector3 operator *(Vector3 left, Vector3 right) => new Vector3(left.x * right.x, left.y * right.y, left.z * right.z);
        public static Vector3 operator *(Vector3 left, float right) => new Vector3(left.x * right, left.y * right, left.z * right);
        public static Vector3 operator *(float left, Vector3 right) => new Vector3(left * right.x, left * right.y, left * right.z);

        public static Vector3 operator /(Vector3 left, Vector3 right) => new Vector3(left.x / right.x, left.y / right.y, left.z / right.z);
        public static Vector3 operator /(Vector3 left, float right) => new Vector3(left.x / right, left.y / right, left.z / right);

        public static bool operator ==(Vector3 left, Vector3 right) => left.x == right.x && left.y == right.y && left.z == right.z;
        public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

        public override bool Equals(object other) => other is Vector3 && this == (Vector3)other;

        public static implicit operator Vector2(Vector3 vec) => new Vector2(vec.x, vec.y);
    }


    public struct Quaternion
    {
        private readonly float[] _data;

        public Quaternion(float w, float x, float y, float z)
        {
            _data = new float[4];
            _data[0] = w;
            _data[1] = x;
            _data[2] = y;
            _data[3] = z;
        }
    }
}