using leopph;
using System;
using System.Runtime.InteropServices;

public class MovementController : Behavior
{
	public Key left = Key.A;
	public Key right = Key.D;
	public Key up = Key.Space;
	public Key down = Key.LeftControl;
	public Key forward = Key.W;
	public Key backward = Key.S;
	public Key run = Key.LeftShift;
	public float speed = 3;

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

		Transform.Translate(speed * posDelta * Time.FrameTime, Space.Local);
	}
}


public class OrientationController : Behavior
{
	[Expose]
	public float sensitivity = 0.05f;

	private void Tick()
	{
		Transform.Rotate(Vector3.Up, sensitivity * Input.MouseDelta.x, Space.World);
		Transform.Rotate(Vector3.Right, sensitivity * Input.MouseDelta.y, Space.Local);
	}
}


public class AppController : Behavior
{
	private Extent2D<uint> _originalResolution;

	private void OnInit()
	{
		_originalResolution = Window.WindowedResolution;
		Input.IsCursorConfined = true;
		Input.IsCursorHidden = true;
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
			Window.WindowedResolution = new Extent2D<uint>(1280, 720);
		}

		if (Input.GetKeyDown(Key.Two))
		{
			Window.WindowedResolution = new Extent2D<uint>(1600, 900);
		}

		if (Input.GetKeyDown(Key.Three))
		{
			Window.WindowedResolution = new Extent2D<uint>(1920, 1080);
		}

		if (Input.GetKeyDown(Key.Zero))
		{
			Window.WindowedResolution = _originalResolution;
		}

		if (Input.GetKeyDown(Key.Q) || Input.GetKeyDown(Key.Escape))
		{
			Application.Quit();
		}
	}
}


public class Test
{
	public static void DoTest()
	{
		var cube0 = new Entity();
		cube0.Name = "Front Cube";
		cube0.Transform.WorldPosition = new Vector3(0, 0, 2);
		cube0.CreateComponent<CubeModel>();

		var cube1 = new Entity();
		cube1.Name = "Right Cube";
		cube1.Transform.WorldPosition = new Vector3(2, 0, 0);
		cube1.CreateComponent<CubeModel>();

		var cube2 = new Entity();
		cube2.Name = "Back Cube";
		cube2.Transform.WorldPosition = new Vector3(0, 0, -2);
		cube2.CreateComponent<CubeModel>();

		var cube3 = new Entity();
		cube3.Name = "Left cube";
		cube3.Transform.WorldPosition = new Vector3(-2, 0, 0);
		cube3.CreateComponent<CubeModel>();

		var cam = new Entity();
		cam.Name = "Camera";
		cam.CreateComponent<Camera>();
		cam.CreateComponent<MovementController>();
		cam.CreateComponent<OrientationController>();

		var util = new Entity();
		util.Name = "Utility";
		util.CreateComponent<AppController>();
	}
}