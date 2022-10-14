using leopph;


public class MovementController : Behavior
{
    public Key left = Key.A;
    public Key right = Key.D;
    public Key up = Key.Space;
    public Key down = Key.LeftControl;
    public Key forward = Key.W;
    public Key backward = Key.S;
    public Key run = Key.LeftShift;

    private void Tick()
    {
        var posDelta = Vector3.Zero;

        if (Input.GetKey(right))
        {
            posDelta += Vector3.Right;
        }

        if (Input.GetKey(left))
        {
            posDelta += Vector3.Left;
        }

        if (Input.GetKey(up))
        {
            posDelta += Vector3.Up;
        }

        if (Input.GetKey(down))
        {
            posDelta += Vector3.Down;
        }

        if (Input.GetKey(forward))
        {
            posDelta += Vector3.Forward;
        }

        if (Input.GetKey(backward))
        {
            posDelta += Vector3.Backward;
        }

        posDelta.Normalize();

        posDelta *= 0.5f;

        if (Input.GetKey(run))
        {
            posDelta *= 2;
        }

        Entity.Translate(posDelta * Time.FrameTime);
    }
}


public class OrientationController : Behavior
{
    private void Tick()
    {
        Entity.Rotate(Vector3.Up, Math.ToDegrees(Input.MouseDelta.x) * Time.FrameTime, Space.World);
        Entity.Rotate(Vector3.Right, Math.ToDegrees(Input.MouseDelta.y) * Time.FrameTime, Space.Object);
    }
}


public class WindowController : Behavior
{
    private Extent2D _originalResolution;

    private void OnInit()
    {
        _originalResolution = Window.WindowedResolution;
    }

    private void Tick()
    {
        if (Input.GetKeyDown(Key.F))
        {
            Window.IsBorderless = !Window.IsBorderless;
        }

        if (Input.GetKeyDown(Key.M))
        {
            Window.IsMinimizingOnBorderlessFocusLoss = !Window.IsMinimizingOnBorderlessFocusLoss;
        }

        if (Input.GetKeyDown(Key.One))
        {
            Window.WindowedResolution = new Extent2D(1280, 720);
        }

        if (Input.GetKeyDown(Key.Two))
        {
            Window.WindowedResolution = new Extent2D(1600, 900);
        }

        if (Input.GetKeyDown(Key.Three))
        {
            Window.WindowedResolution = new Extent2D(1920, 1080);
        }

        if (Input.GetKeyDown(Key.Zero))
        {
            Window.WindowedResolution = _originalResolution;
        }
    }
}


public class Test
{
    public static void DoTest()
    {
        Entity e = new Entity();
        CubeModel c = e.CreateComponent<CubeModel>();

        Entity e2 = new Entity();
        e2.Position = new Vector3(0, 0, -3);
        e2.CreateComponent<Camera>();
        e2.CreateComponent<OrientationController>();
        MovementController m = e2.CreateComponent<MovementController>();

        Entity e3 = new Entity();
        e3.CreateComponent<WindowController>();
    }
}