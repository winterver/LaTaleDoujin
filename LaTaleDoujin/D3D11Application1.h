#pragma once
#include <memory>
#include <tchar.h>

class SysTimer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class D3D11Application1
{
public:
    D3D11Application1(const TCHAR* title, int width, int height);
    ~D3D11Application1();

    bool Init();
    int Run();

protected:
    virtual void InitScene();
    virtual void UpdateScene();
    virtual void DrawScene();

protected:
    // real type of unsigned char is DirectX::Keyboard::Keys
    bool IsKeyDown(unsigned char key);   // true while the key is pressed
    bool IsKeyHit(unsigned char key);    // true at the frame where the key goes from "released" to "pressed"

    SysTimer* GetTimer();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetContext();
    IDXGISwapChain* GetSwapChain();

    ID3D11Texture2D* GetDepthStencilBuffer();
    ID3D11RenderTargetView* GetRenderTargetView();
    ID3D11DepthStencilView* GetDepthStencilView();

private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};
