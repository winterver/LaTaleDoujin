#include "D3D11Application2.h"
#include <cassert>

static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses 
        // a key that does not correspond to any mnemonic or accelerator key. 
    case WM_MENUCHAR:
        // Make it never beep
        return MAKELRESULT(0, MNC_CLOSE);

    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        Mouse::ProcessMessage(msg, wParam, lParam);
        Keyboard::ProcessMessage(msg, wParam, lParam);
        break;

    case WM_KEYUP:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
        Keyboard::ProcessMessage(msg, wParam, lParam);
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_MOUSEHOVER:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        Mouse::ProcessMessage(msg, wParam, lParam);
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

D3D11Application::D3D11Application(const TCHAR* title, int width, int height)
    : mTitle(title), mWidth(width), mHeight(height)
{ }

D3D11Application::~D3D11Application()
{
    if (mContext)
        mContext->ClearState();
}

bool D3D11Application::Init()
{
    bool ret = InitWindow() && InitDirect3D();
    InitScene();
    return ret;
}

int D3D11Application::Run()
{
    MSG msg = { };

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            auto state = mKeyboard->GetState();
            mTracker->Update(state);
            mTimer.Tick();

            UpdateScene();
            DrawScene();
        }
    }

    return (int)msg.wParam;
}

void D3D11Application::InitScene() { }
void D3D11Application::UpdateScene() { }
void D3D11Application::DrawScene() { }

bool D3D11Application::IsKeyDown(Key key)
{
    return mKeyboard->GetState().IsKeyDown(key);
}

bool D3D11Application::IsKeyHit(Key key)
{
    return mTracker->IsKeyPressed(key);
}

bool D3D11Application::InitWindow()
{
    WNDCLASS wc = { };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"D3D11Application";
    wc.lpfnWndProc = WndProc;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if (!RegisterClass(&wc))
        return false;

    RECT rect = { 0, 0, mWidth, mHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    mhWnd = CreateWindow(L"D3D11Application", mTitle, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) - width)/2,
        (GetSystemMetrics(SM_CYSCREEN) - height)/2,
        width, height, 0, 0, wc.hInstance, this);

    if (!mhWnd)
        return false;

    // Disable window resizing
    SetWindowLong(mhWnd, GWL_STYLE, GetWindowLong(mhWnd, GWL_STYLE) & ~(WS_SIZEBOX|WS_MAXIMIZEBOX));

    ShowWindow(mhWnd, SW_SHOW);
    UpdateWindow(mhWnd);

    return true;
}

bool D3D11Application::InitDirect3D()
{
    HRESULT hr;
    UINT createDeviceFlags = 0;

#if defined(_DEBUG)  
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];

        for (UINT featureLevelIndex = 0; featureLevelIndex < numFeatureLevels; featureLevelIndex++)
        {
            hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags,
                featureLevels + featureLevelIndex, numFeatureLevels - featureLevelIndex,
                D3D11_SDK_VERSION, &mDevice, nullptr, &mContext);

            if (SUCCEEDED(hr))
                break;
        }

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return false;

    mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
    assert(m4xMsaaQuality > 0);

    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<IDXGIFactory1> dxgiFactory1; // dx11.0 => dxgi 1.1
    ComPtr<IDXGIFactory2> dxgiFactory2; // dx11.1 => dxgi 1.2

    mDevice.As(&dxgiDevice);
    dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory1));

    dxgiFactory1.As(&dxgiFactory2);
    if (dxgiFactory2)
    {
        DXGI_SWAP_CHAIN_DESC1 swapchainDesc = { };

        swapchainDesc.Width = mWidth;
        swapchainDesc.Height = mHeight;
        swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        if (mEnable4xMsaa)
        {
            swapchainDesc.SampleDesc.Count = 4;
            swapchainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
        }
        else
        {
            swapchainDesc.SampleDesc.Count = 1;
            swapchainDesc.SampleDesc.Quality = 0;
        }

        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.BufferCount = 1;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapchainDesc.Flags = 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;

        fullscreenDesc.RefreshRate.Numerator = 60;
        fullscreenDesc.RefreshRate.Denominator = 1;
        fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fullscreenDesc.Windowed = TRUE;

        ComPtr<IDXGISwapChain1> swapchain1;
        dxgiFactory2->CreateSwapChainForHwnd(mDevice.Get(), mhWnd, &swapchainDesc, &fullscreenDesc, nullptr, &swapchain1);
        swapchain1.As(&mSwapChain);
    }
    else
    {
        DXGI_SWAP_CHAIN_DESC swapchainDesc = { };

        swapchainDesc.BufferDesc.Width = mWidth;
        swapchainDesc.BufferDesc.Height = mHeight;
        swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        if (mEnable4xMsaa)
        {
            swapchainDesc.SampleDesc.Count = 4;
            swapchainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
        }
        else
        {
            swapchainDesc.SampleDesc.Count = 1;
            swapchainDesc.SampleDesc.Quality = 0;
        }

        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.BufferCount = 1;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapchainDesc.Flags = 0;

        swapchainDesc.OutputWindow = mhWnd;
        swapchainDesc.Windowed = TRUE;

        dxgiFactory1->CreateSwapChain(mDevice.Get(), &swapchainDesc, &mSwapChain);
    }

    // Disable Alt+Enter toggle fullscreen.
    dxgiFactory1->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    OnResize();

    mKeyboard = make_unique<Keyboard>();
    mTracker = make_unique<Tracker>();
    mMouse = make_unique<Mouse>();
    mMouse->SetWindow(mhWnd);

    return true;
}

void D3D11Application::OnResize()
{
    assert(mContext);
    assert(mDevice);
    assert(mSwapChain);

    mRenderTargetView.Reset();
    mDepthStencilBuffer.Reset();
    mDepthStencilView.Reset();

    ComPtr<ID3D11Texture2D> backBuffer;
    mSwapChain->ResizeBuffers(1, mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    mSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &mRenderTargetView);

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = mWidth;
    depthStencilDesc.Height = mHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    if (mEnable4xMsaa)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
    }
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    mDevice->CreateTexture2D(&depthStencilDesc, nullptr, &mDepthStencilBuffer);
    mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, &mDepthStencilView);

    // ComPtr::operator& => reference_count--, equivalent to ComPtr::ReleaseAndGetAddressOf
    mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.Width = static_cast<float>(mWidth);
    mViewport.Height = static_cast<float>(mHeight);
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;

    mContext->RSSetViewports(1, &mViewport);
}
