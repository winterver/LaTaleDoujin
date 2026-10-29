using System.Threading.Tasks;
using Vortice.Direct3D11;
using Vortice.DXGI;

namespace MyDXLibrary;

public class GameScene
{
    public GameApplication Game { get; private set; }

    public SystemTimer Timer => Game.Timer;
    public ID3D11Device Device => Game.Device;
    public ID3D11DeviceContext Context => Game.Context;
    public IDXGISwapChain SwapChain => Game.SwapChain;
    public ID3D11RenderTargetView RenderTargetView => Game.RenderTargetView;
    public ID3D11DepthStencilView DepthStencilView => Game.DepthStencilView;

    public GameScene(GameApplication app)
    {
        Game = app;
    }

    internal virtual void InitScene() { }
    internal virtual void UpdateScene() { }
    internal virtual void DrawScene() { }
}
