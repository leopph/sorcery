using leopph;
using System.Runtime.InteropServices;

class Test
{
    [DllImport("RuntimeUnmanaged.dll", EntryPoint = "add_position")]
    private extern static ulong AddPosition(float[] vec);

    [DllImport("RuntimeUnmanaged.dll", EntryPoint = "update_position")]
    private extern static void UpdatePosition(ulong index, float[] vec);


    public Test()
    {
        _id = AddPosition(_pos);
    }

    private void Update()
    {
        float deltaPos = 0.5f;

        if (Input.GetKey(Key.Shift))
        {
            deltaPos *= 2;
        }
        
        deltaPos *= Time.FrameTime;

        if (Input.GetKey(Key.RightArrow))
        {
            _pos[0] += deltaPos;
        }

        if (Input.GetKey(Key.LeftArrow))
        {
            _pos[0] -= deltaPos;
        }

        if (Input.GetKey(Key.Space))
        {
            _pos[1] += deltaPos;
        }

        if (Input.GetKey(Key.Control))
        {
            _pos[1] -= deltaPos;
        }

        if (Input.GetKey(Key.UpArrow))
        {
            _pos[2] += deltaPos;
        }

        if (Input.GetKey(Key.DownArrow))
        {
            _pos[2] -= deltaPos;
        }

        UpdatePosition(_id, _pos);
    }

    private readonly float[] _pos = new float[3] { 0, 0, 0 };
    private readonly ulong _id;
}