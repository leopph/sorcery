using leopph;
using System.Runtime.InteropServices;

public class Test
{
    [DllImport("RuntimeUnmanaged.dll", EntryPoint = "add_position")]
    private extern static ulong AddPosition(Vector3 pos);

    [DllImport("RuntimeUnmanaged.dll", EntryPoint = "update_position")]
    private extern static void UpdatePosition(ulong index, Vector3 pos);


    public Test()
    {
        _id = AddPosition(_pos);
    }

    private void Update()
    {
        var posDelta = Vector3.Zero;

        if (Input.GetKey(Key.RightArrow))
        {
            posDelta += Vector3.Right;
        }

        if (Input.GetKey(Key.LeftArrow))
        {
            posDelta += Vector3.Left;
        }

        if (Input.GetKey(Key.Space))
        {
            posDelta += Vector3.Up;
        }

        if (Input.GetKey(Key.Control))
        {
            posDelta += Vector3.Down;
        }

        if (Input.GetKey(Key.UpArrow))
        {
            posDelta += Vector3.Forward;
        }

        if (Input.GetKey(Key.DownArrow))
        {
            posDelta += Vector3.Backward;
        }

        posDelta.Normalize();

        posDelta *= 0.5f;

        if (Input.GetKey(Key.Shift))
        {
            posDelta *= 2;
        }

        _pos += posDelta * Time.FrameTime;

        UpdatePosition(_id, _pos);
    }

    private Vector3 _pos = Vector3.Zero;
    private readonly ulong _id;
}