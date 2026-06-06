#pragma once
#include <Windows.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include "SysTimer.h"

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class D3D11Application
{
public:
    D3D11Application(const TCHAR* title, int width, int height);
    virtual ~D3D11Application();

    virtual bool Init();
    virtual int Run();

protected:
    virtual void OnResize();
    virtual void UpdateScene();
    virtual void DrawScene();

    static LRESULT _WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool InitWindow();
    bool InitDirect3D();

protected:
    HWND m_hWnd = nullptr;
    bool m_AppPaused = false;
    bool m_Minimized = false;
    bool m_Maximized = false;
    bool m_Resizing = false;
    bool m_Enable4xMsaa = false;
    UINT m_4xMsaaQuality = 0;

    SysTimer m_Timer;

    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    ComPtr<IDXGISwapChain> m_pSwapChain;

    ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
    ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
    D3D11_VIEWPORT m_Viewport = { 0 };

    const TCHAR* m_Title;
    int m_Width;
    int m_Height;
};
