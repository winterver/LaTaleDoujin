#pragma once
#include "D3D11Application.h"
#include <memory>

namespace DirectX
{
    inline namespace DX11
    {
        class SpriteBatch;
        template<class TVertex>
        class PrimitiveBatch;
        class VertexPositionColor;
        class BasicEffect;
        class CommonStates;
    }
    class Keyboard;
    class Mouse;
}

using DirectX::SpriteBatch;
using PrimitiveBatch2 = DirectX::PrimitiveBatch<DirectX::VertexPositionColor>;
using DirectX::Keyboard;
using DirectX::Mouse;
using DirectX::BasicEffect;
using DirectX::CommonStates;

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

    bool Init();

protected:
    void OnResize();
    void UpdateScene();
    void DrawScene();

    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    ComPtr<ID3D11Texture2D> m_Framebuffer1;
    ComPtr<ID3D11Texture2D> m_Framebuffer2;
    ComPtr<ID3D11RenderTargetView> m_Framebuffer1View;
    ComPtr<ID3D11RenderTargetView> m_Framebuffer2View;
    ComPtr<ID3D11ShaderResourceView> m_Framebuffer1Tex;
    ComPtr<ID3D11ShaderResourceView> m_Framebuffer2Tex;

    std::unique_ptr<Keyboard> m_Keyboard;
    std::unique_ptr<Mouse> m_Mouse;

    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11PixelShader> m_GaussianBlur;
    ComPtr<ID3D11Buffer> m_GaussianParametersH;
    ComPtr<ID3D11Buffer> m_GaussianParametersV;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;

    std::unique_ptr<PrimitiveBatch2> m_PrimitiveBatch;
    std::unique_ptr<BasicEffect> m_PrimitiveEffect;
    ComPtr<ID3D11InputLayout> m_PrimitiveLayout;
    std::unique_ptr<CommonStates> m_PrimitiveStates;
};
