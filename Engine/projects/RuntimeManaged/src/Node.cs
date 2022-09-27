using leopph;
using System.Collections.Generic;

public class Node
{
    private string _name = "Node";

    private Vector3 _localPosition = Vector3.Zero;
    private Quaternion _localRotation = Quaternion.Identity;
    private Vector3 _localScale = Vector3.One;

    private Vector3 _worldPosition = Vector3.Zero;
    private Quaternion _worldRotation = Quaternion.Identity;
    private Vector3 _worldScale = Vector3.One;

    private Vector3 _forward = Vector3.Forward;
    private Vector3 _right = Vector3.Right;
    private Vector3 _up = Vector3.Up;

    private Node _parent = null;
    private readonly List<Node> _children = new List<Node>();

    Matrix4 _modelMat = Matrix4.Identity;
    Matrix3 _normalMat = Matrix3.Identity;
}