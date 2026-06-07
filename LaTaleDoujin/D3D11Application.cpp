#include "D3D11Application.h"
#include <cassert>
#include <iostream>
#include "LaTaleDoujin.h"

extern "C"
{
    // Prioritize NVIDIA and AMD GPUs over others.
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

D3D11Application::D3D11Application(const TCHAR* title, int width, int height)
    : m_Title(title)
    , m_Width(width)
    , m_Height(height)
{
}

D3D11Application::~D3D11Application() = default;

bool D3D11Application::Init()
{
    return InitWindow() && InitDirect3D();
}

int D3D11Application::Run()
{
    MSG msg = { };
    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            m_Timer.Tick();

            if (!m_AppPaused)
            {
                UpdateScene();
                DrawScene();
            }
            else
            {
                m_Timer.Reset();
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

void D3D11Application::OnResize()
{
    assert(m_pContext);
    assert(m_pDevice);
    assert(m_pSwapChain);

    ComPtr<ID3D11Texture2D> backBuffer;
    m_pSwapChain->ResizeBuffers(1, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    m_pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pRenderTargetView);

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = m_Width;
    depthStencilDesc.Height = m_Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    if (m_Enable4xMsaa)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
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

    m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
    m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, &m_pDepthStencilView);

    // ComPtr::operator& => reference_count--, equivalent to ComPtr::ReleaseAndGetAddressOf
    m_pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = static_cast<float>(m_Width);
    m_Viewport.Height = static_cast<float>(m_Height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_pContext->RSSetViewports(1, &m_Viewport);
}

void D3D11Application::UpdateScene()
{
}

void D3D11Application::DrawScene()
{
}

LRESULT D3D11Application::_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPCREATESTRUCT lpCreateStruct = (LPCREATESTRUCT)lParam;
    D3D11Application* app = (D3D11Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE:
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpCreateStruct->lpCreateParams);
        return 0;

    default:
        return app ? app->WndProc(hWnd, msg, wParam, lParam) : DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

LRESULT D3D11Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
        }
        else
        {
            m_AppPaused = false;
        }
        return 0;

    case WM_SIZE:
        m_Width = LOWORD(lParam);
        m_Height = HIWORD(lParam);
        if (m_pDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (m_Resizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        return 0;

    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        OnResize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses 
        // a key that does not correspond to any mnemonic or accelerator key. 
    case WM_MENUCHAR:
        // Make it never beep
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        return 0;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        return 0;

    case WM_MOUSEMOVE:
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool D3D11Application::InitWindow()
{
    WNDCLASS wc = { };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"D3D11Application";
    wc.lpfnWndProc = _WndProc;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if (!RegisterClass(&wc))
        return false;

    RECT rect = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    m_hWnd = CreateWindow(L"D3D11Application", m_Title, WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) - width)/2,
        (GetSystemMetrics(SM_CYSCREEN) - height)/2,
        width, height, 0, 0, wc.hInstance, this);

    if (!m_hWnd)
        return false;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

bool D3D11Application::InitDirect3D()
{
    HRESULT hr;
    UINT createDeviceFlags;

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
                D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pContext);

            if (SUCCEEDED(hr))
                break;
        }

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return false;

    m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
    assert(m_4xMsaaQuality > 0);

    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<IDXGIFactory1> dxgiFactory1; // dx11.0 => dxgi 1.1
    ComPtr<IDXGIFactory2> dxgiFactory2; // dx11.1 => dxgi 1.2

    m_pDevice.As(&dxgiDevice);
    dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf()));

    dxgiFactory1.As(&dxgiFactory2);
    if (dxgiFactory2)
    {
        DXGI_SWAP_CHAIN_DESC1 swapchainDesc = { };

        swapchainDesc.Width = m_Width;
        swapchainDesc.Height = m_Height;
        swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        if (m_Enable4xMsaa)
        {
            swapchainDesc.SampleDesc.Count = 4;
            swapchainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
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
        dxgiFactory2->CreateSwapChainForHwnd(m_pDevice.Get(), m_hWnd, &swapchainDesc, &fullscreenDesc, nullptr, &swapchain1);
        swapchain1.As(&m_pSwapChain);
    }
    else
    {
        DXGI_SWAP_CHAIN_DESC swapchainDesc = { };

        swapchainDesc.BufferDesc.Width = m_Width;
        swapchainDesc.BufferDesc.Height = m_Height;
        swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        if (m_Enable4xMsaa)
        {
            swapchainDesc.SampleDesc.Count = 4;
            swapchainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
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

        swapchainDesc.OutputWindow = m_hWnd;
        swapchainDesc.Windowed = TRUE;

        dxgiFactory1->CreateSwapChain(m_pDevice.Get(), &swapchainDesc, &m_pSwapChain);
    }

    // Disable Alt+Enter toggle fullscreen.
    dxgiFactory1->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    OnResize();

    return true;
}
