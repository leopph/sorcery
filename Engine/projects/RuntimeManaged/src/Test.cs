using leopph;
using System;
using System.Runtime.InteropServices;

public class Cube : Node
{
    [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "add_position")]
    private extern static ulong AddPosition(in Vector3 pos);

    [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "update_position")]
    private extern static void UpdatePosition(ulong index, in Vector3 pos);


    public Cube()
    {
        _id = AddPosition(Position);
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

        Position += posDelta * Time.FrameTime;

        UpdatePosition(_id, Position);
    }

    private readonly ulong _id;
}

public class Camera : Node
{
    [DllImport(Constants.UNMANAGED_DLL_NAME, EntryPoint = "set_cam_pos")]
    private static extern void SetCamPos(in Vector3 pos);

    public Camera()
    {
        Position = new Vector3(0, 0, -2);
        SetCamPos(Position);
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

        Position += posDelta * Time.FrameTime;

        SetCamPos(Position);
    }
}