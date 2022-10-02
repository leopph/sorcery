using leopph;
using System.Runtime.InteropServices;

public class Cube : MonoDynamicNode
{
    [DllImport("leopph_runtime_native.dll", EntryPoint = "add_position")]
    private extern static ulong AddPosition(ref Vector3 pos);

    [DllImport("leopph_runtime_native.dll", EntryPoint = "update_position")]
    private extern static void UpdatePosition(ulong index, ref Vector3 pos);


    public Cube()
    {
        _id = AddPosition(ref _pos);
    }

    private void Tick()
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

        if (Input.GetKey(Key.RightAlt))
        {
            posDelta += Vector3.Up;
        }

        if (Input.GetKey(Key.RightControl))
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

        UpdatePosition(_id, ref _pos);
    }

    private Vector3 _pos = Vector3.Zero;
    private readonly ulong _id;
}

public class Camera : MonoDynamicNode
{
    [DllImport("leopph_runtime_native.dll", EntryPoint = "set_cam_pos")]
    private static extern void SetCamPos(ref Vector3 pos);

    public Camera()
    {
        SetCamPos(ref _pos);
    }

    private void Tick()
    {
        var posDelta = Vector3.Zero;

        if (Input.GetKey(Key.D))
        {
            posDelta += Vector3.Right;
        }

        if (Input.GetKey(Key.A))
        {
            posDelta += Vector3.Left;
        }

        if (Input.GetKey(Key.Space))
        {
            posDelta += Vector3.Up;
        }

        if (Input.GetKey(Key.LeftControl))
        {
            posDelta += Vector3.Down;
        }

        if (Input.GetKey(Key.W))
        {
            posDelta += Vector3.Forward;
        }

        if (Input.GetKey(Key.S))
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

        SetCamPos(ref _pos);
    }

    private Vector3 _pos = new Vector3(0, 0, -2);
}
