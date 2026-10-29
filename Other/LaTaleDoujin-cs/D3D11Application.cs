using Windows.Win32.Foundation;
using Windows.Win32.Graphics.Gdi;
using Windows.Win32.UI.WindowsAndMessaging;
using static Windows.Win32.UI.WindowsAndMessaging.WNDCLASS_STYLES;
using static Windows.Win32.Graphics.Gdi.GET_STOCK_OBJECT_FLAGS;
using static Windows.Win32.UI.WindowsAndMessaging.WINDOW_STYLE;
using static Windows.Win32.UI.WindowsAndMessaging.SYSTEM_METRICS_INDEX;
using static Windows.Win32.UI.WindowsAndMessaging.SHOW_WINDOW_CMD;
using static Windows.Win32.UI.WindowsAndMessaging.PEEK_MESSAGE_REMOVE_TYPE;
using static Windows.Win32.System.Com.COINIT; 
using static Windows.Win32.PInvoke;

using Vortice.Direct3D11;
using Vortice.Direct3D;
using Vortice.DXGI;
using Vortice.Mathematics;
using static Vortice.Direct3D11.D3D11;

using SharpGen.Runtime;
using System.Diagnostics;
using System;

namespace MyDXLibrary;

public class D3D11Application : IDisposable
{
    public string Title { get; private set; }
    public int Width { get; private set; }
    public int Height { get; private set; }
    public nint Hwnd { get; private set; }
    public uint MsaaQuality { get; private set; }

    private ID3D11Device _device;
    private ID3D11DeviceContext _context;
    private IDXGISwapChain _swapchain;

    public ID3D11Device Device => _device;
    public ID3D11DeviceContext Context => _context;
    public IDXGISwapChain SwapChain => _swapchain;

    public ID3D11RenderTargetView RenderTargetView { get; private set; }
    public ID3D11DepthStencilView DepthStencilView { get; private set; }

    public SystemTimer Timer { get; private set; } = new();

    public D3D11Application(string title, int width, int height)
    {
        Title = title;
        Width = width;
        Height = height;
    }

    public bool Init()
    {
        var b1 = InitWindow();
        var b2 = InitDirect3D();
        HRESULT hr = CoInitializeEx(COINIT_MULTITHREADED);

        InitScene();

        return b1 & b2 & hr.Succeeded;
    }

    public int Run()
    {
        MSG msg = default;

        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(out msg, default, 0, 0, PM_REMOVE))
            {
                TranslateMessage(msg);
                DispatchMessage(msg);
            }
            else
            {
                Timer.Tick();
                UpdateScene();
                DrawScene();
            }
        }

        Dispose();
        return (int)msg.wParam.Value;
    }

    protected virtual void InitScene() { }
    protected virtual void UpdateScene() { }
    protected virtual void DrawScene() { }

    public void Dispose()
    {
        Context?.ClearState();
        DepthStencilView?.Dispose();
        RenderTargetView?.Dispose();
        SwapChain?.Dispose();
        Context?.Dispose();
        Device?.Dispose();
    }

    private unsafe bool InitWindow()
    {
        PCWSTR lpszClassName;
        fixed (char* className = "D3D11Application")
        {
            lpszClassName = new PCWSTR(className);
        }

        PCWSTR title;
        fixed (char* title2 = Title)
        {
            title = new PCWSTR(title2);
        }

        WNDCLASSW wc = default;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpszClassName = lpszClassName;
        wc.lpfnWndProc = WndProc;

        wc.hCursor = LoadCursor(default, IDC_ARROW);
        wc.hbrBackground = new HBRUSH(GetStockObject(BLACK_BRUSH).Value);
        wc.hInstance = new HINSTANCE(GetModuleHandle(null).DangerousGetHandle());

        if (RegisterClass(in wc) == 0)
            return false;

        RECT rect = new(0, 0, Width, Height);
        AdjustWindowRect(ref rect, WS_OVERLAPPEDWINDOW, false);

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        int x = (GetSystemMetrics(SM_CXSCREEN) - width)/2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - height)/2;

        var style = WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX|WS_MAXIMIZEBOX);
        Hwnd = CreateWindowEx(default, lpszClassName, title, style,
            x, y, width, height, default, default, wc.hInstance, null);

        if (Hwnd == 0)
            return false;

        ShowWindow(new HWND(Hwnd), SW_SHOW);
        UpdateWindow(new HWND(Hwnd));

        return true;
    }

    private static LRESULT WndProc(HWND hWnd, uint message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return default;

            // The WM_MENUCHAR message is sent when a menu is active and the user presses 
            // a key that does not correspond to any mnemonic or accelerator key. 
        case WM_MENUCHAR:
            // Make it never beep
            return MAKELRESULT(0, (ushort)MNC_CLOSE);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    private bool InitDirect3D()
    {
        DriverType[] driverTypes = {
            DriverType.Hardware,
            DriverType.Warp,
            DriverType.Reference,
        };
        int numDriverTypes = driverTypes.Length;

        FeatureLevel[] featureLevels =
        {
            FeatureLevel.Level_11_1,
            FeatureLevel.Level_11_0,
        };
        int numFeatureLevels = featureLevels.Length;

        DeviceCreationFlags createDeviceFlags = default;
#if DEBUG
        createDeviceFlags = DeviceCreationFlags.Debug;
#endif

        Result hr = default;

        for (int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            DriverType driverType = driverTypes[driverTypeIndex];

            for (int featureLevelIndex = 0; featureLevelIndex < numFeatureLevels; featureLevelIndex++)
            {
                FeatureLevel[] featureLevels1 = featureLevels[featureLevelIndex..];
                FeatureLevel featureLevel;

                hr = D3D11CreateDevice(null, driverType, createDeviceFlags, featureLevels1,
                    out _device, out featureLevel, out _context);

                if (hr.Success)
                    break;
            }

            if (hr.Success)
                break;
        }

        if (hr.Failure)
            return false;

        MsaaQuality = Device.CheckMultisampleQualityLevels(Format.R8G8B8A8_UNorm, 4);
        Debug.Assert(MsaaQuality > 0);

        IDXGIDevice dxgiDevice = Device.QueryInterface<IDXGIDevice>();
        IDXGIAdapter dxgiAdapter = dxgiDevice.GetAdapter();
        IDXGIFactory1 dxgiFactory1 = dxgiAdapter.GetParent<IDXGIFactory1>();
        IDXGIFactory2 dxgiFactory2 = dxgiFactory1.QueryInterfaceOrNull<IDXGIFactory2>();

        if (dxgiFactory2 != null)
        {
            SwapChainDescription1 swapchainDesc1 = default;
            swapchainDesc1.Width = (uint)Width;
            swapchainDesc1.Height = (uint)Height;
            swapchainDesc1.Format = Format.R8G8B8A8_UNorm;

            swapchainDesc1.SampleDescription.Count = 4;
            swapchainDesc1.SampleDescription.Quality = MsaaQuality - 1;

            swapchainDesc1.BufferUsage = Usage.RenderTargetOutput;
            swapchainDesc1.BufferCount = 1;
            swapchainDesc1.SwapEffect = SwapEffect.Discard;
            swapchainDesc1.Flags = 0;

            SwapChainFullscreenDescription fullscreenDesc = default;
            fullscreenDesc.RefreshRate.Numerator = 60;
            fullscreenDesc.RefreshRate.Denominator = 1;
            fullscreenDesc.Scaling = ModeScaling.Unspecified;
            fullscreenDesc.ScanlineOrdering = ModeScanlineOrder.Unspecified;
            fullscreenDesc.Windowed = true;

            IDXGISwapChain1 swapchain1 = dxgiFactory2.CreateSwapChainForHwnd(Device, Hwnd, swapchainDesc1, fullscreenDesc);
            _swapchain = swapchain1.QueryInterface<IDXGISwapChain>();
        }
        else
        {
            SwapChainDescription swapchainDesc = default;

            swapchainDesc.BufferDescription.Width = (uint)Width;
            swapchainDesc.BufferDescription.Height = (uint)Height;
            swapchainDesc.BufferDescription.Format = Format.R8G8B8A8_UNorm;

            swapchainDesc.BufferDescription.RefreshRate.Numerator = 60;
            swapchainDesc.BufferDescription.RefreshRate.Denominator = 1;
            swapchainDesc.BufferDescription.Scaling = ModeScaling.Unspecified;
            swapchainDesc.BufferDescription.ScanlineOrdering = ModeScanlineOrder.Unspecified;

            swapchainDesc.SampleDescription.Count = 4;
            swapchainDesc.SampleDescription.Quality = MsaaQuality - 1;

            swapchainDesc.BufferUsage = Usage.RenderTargetOutput;
            swapchainDesc.BufferCount = 1;
            swapchainDesc.SwapEffect = SwapEffect.Discard;
            swapchainDesc.Flags = 0;

            swapchainDesc.OutputWindow = Hwnd;
            swapchainDesc.Windowed = true;

            _swapchain = dxgiFactory1.CreateSwapChain(Device, swapchainDesc);
        }

        dxgiFactory1.MakeWindowAssociation(Hwnd, WindowAssociationFlags.IgnoreAltEnter);

        OnResize();

        return true;
    }

    private void OnResize()
    {
        Debug.Assert(Device != null);
        Debug.Assert(Context != null);
        Debug.Assert(SwapChain != null);

        RenderTargetView?.Dispose();
        DepthStencilView?.Dispose();
        RenderTargetView = null;
        DepthStencilView = null;

        SwapChain.ResizeBuffers(1, (uint)Width, (uint)Height, Format.R8G8B8A8_UNorm, 0);
        ID3D11Texture2D backBuffer = SwapChain.GetBuffer<ID3D11Texture2D>(0);
        RenderTargetView = Device.CreateRenderTargetView(backBuffer);

        Texture2DDescription depthStencilDesc = default;
        depthStencilDesc.Width = (uint)Width;
        depthStencilDesc.Height = (uint)Height;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = Format.D24_UNorm_S8_UInt;

        depthStencilDesc.SampleDescription.Count = 4;
        depthStencilDesc.SampleDescription.Quality = MsaaQuality - 1;

        depthStencilDesc.Usage = ResourceUsage.Default;
        depthStencilDesc.BindFlags = BindFlags.DepthStencil;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;

        var buffer = Device.CreateTexture2D(depthStencilDesc);
        DepthStencilView = Device.CreateDepthStencilView(buffer);

        Context.OMSetRenderTargets(RenderTargetView, DepthStencilView);

        Viewport viewport = default;
        viewport.X = 0;
        viewport.Y = 0;
        viewport.Width = Width;
        viewport.Height = Height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        Context.RSSetViewport(viewport);
    }
}