#include "LaTaleDoujin.h"
#include <SimpleMath.h>
#include <Keyboard.h>
#include <WICTextureLoader.h>
#include "SysTimer.h"
#include "PhysicsSystem.h"
#include "Player.h"
#include "DebugBatch.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;
using Keys = Keyboard::Keys;

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application1(L"La Tale Doujin", 1600, 900)
{ }

LaTaleDoujin::~LaTaleDoujin() = default;

void LaTaleDoujin::InitScene()
{
    m_CommonStates = std::make_unique<CommonStates>(GetDevice());
    m_SpriteBatch = std::make_unique<SpriteBatch>(GetContext());

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    ThrowIfFailed(CreateWICTextureFromFile(GetDevice(), nullptr, L"../Resource/IRIS.PNG", nullptr, &m_IrisTexture));

    m_PhysicsSystem = std::make_unique<PhysicsSystem>();
    auto ground1 = m_PhysicsSystem->CreatePlatform(Vector2(0, 825), Vector2(1600, 50));
    auto ground2 = m_PhysicsSystem->CreatePlatform(Vector2(800, 660), Vector2(800, 90), true);
    auto ground3 = m_PhysicsSystem->CreatePlatform(Vector2(1200, 50), Vector2(50, 800));
    auto ground4 = m_PhysicsSystem->CreatePlatform(Vector2(-10, 0), Vector2(10, 900));

    auto playerEntity = m_PhysicsSystem->CreateEntity(Vector2(200, 450), Vector2(32, 75));
    m_Player = std::make_unique<Player>(playerEntity);

    m_DebugBatch = std::make_unique<DebugBatch>(GetDevice());
    m_DebugBatch->PutSolidRect(ground1->Position, ground1->Size);
    m_DebugBatch->PutHollowRect(ground2->Position, ground2->Size, Vector4(0, 0, 1, 1));
    m_DebugBatch->PutHollowRect(ground3->Position, ground3->Size);
    m_DebugBatch->UpdateScene();
}

void LaTaleDoujin::UpdateScene()
{
    m_Player->Update();
    if (IsKeyDown(Keys::Right)) { m_Player->MoveRight(); }
    if (IsKeyDown(Keys::Left)) { m_Player->MoveLeft(); }
    if (IsKeyHit(Keys::Space)) { m_Player->Jump(IsKeyDown(Keys::Down)); }

    m_PhysicsSystem->Update(GetTimer()->DeltaTime());
}

void LaTaleDoujin::DrawScene()
{
    GetContext()->ClearRenderTargetView(GetRenderTargetView(), Colors::CornflowerBlue);
    GetContext()->ClearDepthStencilView(GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    static float a = 0;
    //a += m_Timer.DeltaTime();

    float cx = 1600 / 2;
    float cy = 900 / 2;

    float x = cx - (cx * cos(a) - cy * sin(a));
    float y = cy - (cx * sin(a) + cy * cos(a));

    auto player = m_Player->GetEntity();

    auto view = XMMatrixLookToLH(
        player->Position - Vector2(800, 450) + Vector3(x, y, 0),
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

    GetContext()->OMSetBlendState(m_CommonStates->Opaque(), nullptr, 0xFFFFFFFF);
    GetContext()->OMSetDepthStencilState(m_CommonStates->DepthDefault(), 0);
    GetContext()->RSSetState(m_CommonStates->CullNone());
    m_DebugBatch->DrawScene(view);
    m_DebugBatch->DrawHollowSprite(player->Position - player->HalfSize, player->HalfSize * 2, Vector4(1, 1, 1, 1), 0, view);

    GetSwapChain()->Present(0, 0);
}
