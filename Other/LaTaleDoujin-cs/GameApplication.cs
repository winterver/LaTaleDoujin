using System;
using System.Threading.Tasks;

namespace MyDXLibrary;

public class GameApplication : D3D11Application
{
    private GameScene _current;
    private GameScene _next;
    private Task _task;

    public GameApplication(string title, int width, int height) : base(title, width, height)
    { }

    protected void ChangeScene(GameScene scene)
    {
        _next = scene;
    }

    protected override void UpdateScene()
    {
        if (_next != null)
        {
            if (_task == null)
            {
                _task = Task.Run(() => _next.InitScene());
            }
            else if (_task.IsCompleted)
            {
                _current = _next;
                _next = null;
                _task = null;
                GC.Collect();
            }
        }

        _current?.UpdateScene();       
    }

    protected override void DrawScene()
    {
        _current?.DrawScene();       
    }
}