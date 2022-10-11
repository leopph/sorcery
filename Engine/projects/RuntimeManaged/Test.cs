using leopph;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


public class Cube : Behavior
{
    ulong _index;

    private void OnInit()
    {
        _index = InternalAddPos(Entity.Position);
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

        Entity.Translate(posDelta * Time.FrameTime);
        InternalUpdatePos(_index, Entity.Position);
    }


    [MethodImpl(MethodImplOptions.InternalCall)]
    private extern static ulong InternalAddPos(in Vector3 pos);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private extern static void InternalUpdatePos(ulong index, in Vector3 pos);
}


public class Camera : Behavior
{
    [DllImport("LeopphRuntimeNative.dll", EntryPoint = "set_cam_pos")]
    private static extern void SetCamPos(in Vector3 pos);

    private void OnInit()
    {
        Entity.Position = new Vector3(0, 0, -3);
        SetCamPos(Entity.Position);
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

        Entity.Translate(posDelta * Time.FrameTime);

        SetCamPos(Entity.Position);
    }
}


public class Test
{
    public static void DoTest()
    {
        Entity e = new Entity();
        e.CreateBehavior<Cube>();

        Entity e2 = new Entity();
        e2.CreateBehavior<Camera>();
    }
}