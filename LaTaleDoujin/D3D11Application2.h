#pragma once
#include <Windows.h>
#include <d3d11_1.h>
#include "Utility.h"
#include "SysTimer.h"
#include <Keyboard.h>
#include <Mouse.h>

using DirectX::Keyboard;
using DirectX::Mouse;
using Key = Keyboard::Keys;
using Tracker = Keyboard::KeyboardStateTracker;

class D3D11Application
{
public:
    D3D11Application(const TCHAR* title, int width, int height);
    ~D3D11Application();

    bool Init();
    int Run();

protected:
    virtual void InitScene();
    virtual void UpdateScene();
    virtual void DrawScene();

    bool IsKeyDown(Key key);
    bool IsKeyHit(Key key);

private:
    bool InitWindow();
    bool InitDirect3D();
    void OnResize();

protected:
    const TCHAR* mTitle;
    int mWidth;
    int mHeight;

    HWND mhWnd = nullptr;
    UINT m4xMsaaQuality = 0;
    bool mEnable4xMsaa = true;

    SysTimer mTimer;

    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;
    ComPtr<IDXGISwapChain> mSwapChain;

    ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
    ComPtr<ID3D11RenderTargetView> mRenderTargetView;
    ComPtr<ID3D11DepthStencilView> mDepthStencilView;
    D3D11_VIEWPORT mViewport = { };

    unique_ptr<Keyboard> mKeyboard;
    unique_ptr<Tracker> mTracker;
    unique_ptr<Mouse> mMouse;
};

