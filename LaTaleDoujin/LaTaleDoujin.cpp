#include "LaTaleDoujin.h"
#include "Utility.h"
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Effects.h>
#include <CommonStates.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1360, 768)
{
}

LaTaleDoujin::~LaTaleDoujin() = default;

bool LaTaleDoujin::Init()
{
    bool val = D3D11Application::Init();
    if (!val) return false;

    m_Keyboard = std::make_unique<Keyboard>();
    m_Mouse = std::make_unique<Mouse>();
    m_Mouse->SetWindow(m_hWnd);

    m_CommonStates = std::make_unique<CommonStates>(m_pDevice.Get());
    m_SpriteBatch = std::make_unique<SpriteBatch>(m_pContext.Get());

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    CreateTextureFromFile(m_pDevice.Get(), nullptr, L"C:/Data/Develop/archive/LaTaleDoujin_CSharp_SDL/LaTaleDoujin/resources/IRIS.PNG", &m_IrisTexture);

    m_PrimitiveBatch = std::make_unique<PrimitiveBatch2>(m_pContext.Get());
    m_PrimitiveEffect = std::make_unique<BasicEffect>(m_pDevice.Get());
    m_PrimitiveEffect->SetProjection(XMMatrixOrthographicOffCenterRH(0, m_Width, m_Height, 0, 0, 1));
    m_PrimitiveEffect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;
    m_PrimitiveEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    ThrowIfFailed(m_pDevice->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        shaderByteCode,
        byteCodeLength,
        &m_PrimitiveLayout));

    return true;
}

void LaTaleDoujin::OnResize()
{
    D3D11Application::OnResize();

    if (m_PrimitiveEffect)
        m_PrimitiveEffect->SetProjection(XMMatrixOrthographicOffCenterRH(0, m_Width, m_Height, 0, 0, 1));
}

void LaTaleDoujin::UpdateScene()
{
}

void LaTaleDoujin::DrawScene()
{
    static float cornflowerblue[] = { 100.0f/255, 149.0f/255, 237.0f/255, 1.0f };
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), cornflowerblue);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto keys = m_Keyboard->GetState();
    auto buttons = m_Mouse->GetState();

    m_SpriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, m_CommonStates->DepthDefault());
    m_SpriteBatch->Draw(m_IrisTexture.Get(), XMFLOAT2(0, 0), nullptr, Colors::White, 0, XMFLOAT2(), 1, SpriteEffects_None, 0.0);
    m_SpriteBatch->End();

    m_SpriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, m_CommonStates->DepthDefault());
    m_SpriteBatch->Draw(m_IrisTexture.Get(), XMFLOAT2(100, 100), nullptr, Colors::White, 0, XMFLOAT2(), 1, SpriteEffects_None, 0.5);
    m_SpriteBatch->End();

    {
        m_pContext->OMSetBlendState(m_CommonStates->Opaque(), nullptr, 0xFFFFFFFF);
        m_pContext->OMSetDepthStencilState(m_CommonStates->DepthNone(), 0);
        m_pContext->RSSetState(m_CommonStates->CullNone());

        m_PrimitiveEffect->Apply(m_pContext.Get());
        m_pContext->IASetInputLayout(m_PrimitiveLayout.Get());

        m_PrimitiveBatch->Begin();
        m_PrimitiveBatch->DrawLine(
            VertexPositionColor(XMFLOAT3(0, 0, 0), XMFLOAT4(1, 1, 1, 1)),
            VertexPositionColor(XMFLOAT3(100, 100, 0), XMFLOAT4(1, 1, 1, 1)));
        m_PrimitiveBatch->End();
    }

    m_pSwapChain->Present(0, 0);
}

LRESULT LaTaleDoujin::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(msg, wParam, lParam);
        Mouse::ProcessMessage(msg, wParam, lParam);
        break;

    case WM_SYSKEYDOWN:
        Keyboard::ProcessMessage(msg, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard::ProcessMessage(msg, wParam, lParam);
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(msg, wParam, lParam);
        break;
    }
    return D3D11Application::WndProc(hWnd, msg, wParam, lParam);
}
