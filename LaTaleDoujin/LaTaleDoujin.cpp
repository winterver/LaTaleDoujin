#include "LaTaleDoujin.h"
#include "Utility.h"
#include "DebugBatch.h"
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <CommonStates.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1360, 768)
{
    m_Enable4xMsaa = true;
    m_EnablePause = false;
}

LaTaleDoujin::~LaTaleDoujin() = default;

bool LaTaleDoujin::Init()
{
    bool val = D3D11Application::Init();
    if (!val) return false;

    // Disable window resizing
    SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(WS_SIZEBOX|WS_MAXIMIZEBOX));

    m_Keyboard = std::make_unique<Keyboard>();
    m_Mouse = std::make_unique<Mouse>();
    m_Mouse->SetWindow(m_hWnd);

    m_CommonStates = std::make_unique<CommonStates>(m_pDevice.Get());
    m_SpriteBatch = std::make_unique<SpriteBatch>(m_pContext.Get());

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    CreateTextureFromFile(m_pDevice.Get(), nullptr, L"C:/Data/Develop/archive/LaTaleDoujin_CSharp_SDL/LaTaleDoujin/resources/IRIS.PNG", &m_IrisTexture);

    m_DebugBatch = std::make_unique<DebugBatch>(m_pDevice.Get());
    m_DebugBatch->PutHollowRect(Vector2(0, 0), Vector2(100, 100));
    m_DebugBatch->PutSolidRect(Vector2(100, 100), Vector2(100, 100));
    m_DebugBatch->UpdateScene();

    return true;
}

void LaTaleDoujin::UpdateScene()
{
}

void LaTaleDoujin::DrawScene()
{
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::CornflowerBlue);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto keys = m_Keyboard->GetState();
    auto buttons = m_Mouse->GetState();

    static float a = 0;
    a += m_Timer.DeltaTime();

    float cx = m_Width / 2;
    float cy = m_Height / 2;

    float x = cx - (cx * cos(a) - cy * sin(a));
    float y = cy - (cx * sin(a) + cy * cos(a));

    auto view = XMMatrixLookToLH(
        Vector3(x, y, 0),
        Vector3(0, 0, 1),
        Vector3(sin(-a), cos(-a), 0));

    m_SpriteBatch->Begin(
        SpriteSortMode_Deferred,
        m_CommonStates->AlphaBlend(),
        m_CommonStates->LinearWrap(),
        m_CommonStates->DepthDefault(),
        m_CommonStates->CullNone(),
        nullptr, view);

    m_SpriteBatch->Draw(m_IrisTexture.Get(), Vector2(0, 0), nullptr, Colors::White, 0, Vector2(), 1, SpriteEffects_None, 0);
    m_SpriteBatch->Draw(m_IrisTexture.Get(), Vector2(100, 100), nullptr, Colors::White, 0, Vector2(), 1, SpriteEffects_None, 0);

    m_SpriteBatch->End();

    m_pContext->OMSetBlendState(m_CommonStates->Opaque(), nullptr, 0xFFFFFFFF);
    m_pContext->OMSetDepthStencilState(m_CommonStates->DepthDefault(), 0);
    m_pContext->RSSetState(m_CommonStates->CullNone());
    m_DebugBatch->DrawScene(view);
    m_DebugBatch->DrawHollowSprite(Vector2(200, 200), Vector2(100, 100), Vector4(1, 0, 0, 1), 0, view);
    m_DebugBatch->DrawSolidSprite(Vector2(300, 300), Vector2(100, 100), Vector4(1, 0, 0, 1), 0, view);

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
    return D3D11Application::WndProc(hWnd, msg, wParam, lParam);
}
