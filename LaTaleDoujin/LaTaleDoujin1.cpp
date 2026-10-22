#include "LaTaleDoujin1.h"
#include <SimpleMath.h>
#include <WICTextureLoader.h>
#include "PhysicsSystem.h"
#include "Player.h"
#include "DebugBatch.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1600, 900)
{ }

LaTaleDoujin::~LaTaleDoujin() = default;

void LaTaleDoujin::InitScene()
{
    mCommonStates = make_unique<CommonStates>(mDevice.Get());
    mSpriteBatch = make_unique<SpriteBatch>(mContext.Get());

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    ThrowIfFailed(CreateWICTextureFromFile(mDevice.Get(), nullptr, L"../Resource/IRIS.PNG", nullptr, &mIrisTexture));

    mPhysicsSystem = make_unique<PhysicsSystem>();
    auto ground1 = mPhysicsSystem->CreatePlatform(Vector2(0, 825), Vector2(1600, 50));
    auto ground2 = mPhysicsSystem->CreatePlatform(Vector2(800, 660), Vector2(800, 90), true);
    auto ground3 = mPhysicsSystem->CreatePlatform(Vector2(1200, 50), Vector2(50, 800));
    auto ground4 = mPhysicsSystem->CreatePlatform(Vector2(-10, 0), Vector2(10, 900));

    auto playerEntity = mPhysicsSystem->CreateEntity(Vector2(200, 450), Vector2(32, 75));
    mPlayer = make_unique<Player>(playerEntity);

    mDebugBatch = make_unique<DebugBatch>(mDevice.Get());
    mDebugBatch->PutSolidRect(ground1->Position, ground1->Size);
    mDebugBatch->PutHollowRect(ground2->Position, ground2->Size, Vector4(0, 0, 1, 1));
    mDebugBatch->PutHollowRect(ground3->Position, ground3->Size);
    mDebugBatch->UpdateScene();
}

void LaTaleDoujin::UpdateScene()
{
    mPlayer->Update();
    if (IsKeyDown(Key::Right)) { mPlayer->MoveRight(); }
    if (IsKeyDown(Key::Left)) { mPlayer->MoveLeft(); }
    if (IsKeyHit(Key::Space)) { mPlayer->Jump(IsKeyDown(Key::Down)); }

    mPhysicsSystem->Update(mTimer.DeltaTime());
}

void LaTaleDoujin::DrawScene()
{
    mContext->ClearRenderTargetView(mRenderTargetView.Get(), Colors::CornflowerBlue);
    mContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    static float a = 0;
    //a += mTimer.DeltaTime();

    float cx = 1600 / 2;
    float cy = 900 / 2;

    float x = cx - (cx * cos(a) - cy * sin(a));
    float y = cy - (cx * sin(a) + cy * cos(a));

    auto player = mPlayer->GetEntity();

    auto view = XMMatrixLookToLH(
        player->Position - Vector2(800, 450) + Vector3(x, y, 0),
        Vector3(0, 0, 1),
        Vector3(sin(-a), cos(-a), 0));

    mSpriteBatch->Begin(
        SpriteSortMode_Deferred,
        mCommonStates->AlphaBlend(),
        mCommonStates->LinearWrap(),
        mCommonStates->DepthDefault(),
        mCommonStates->CullNone(),
        nullptr, view);

    mSpriteBatch->Draw(mIrisTexture.Get(), Vector2(0, 0), nullptr, Colors::White, 0, Vector2(), 1, SpriteEffects_None, 0);
    mSpriteBatch->Draw(mIrisTexture.Get(), Vector2(100, 100), nullptr, Colors::White, 0, Vector2(), 1, SpriteEffects_None, 0);

    mSpriteBatch->End();

    mContext->OMSetBlendState(mCommonStates->Opaque(), nullptr, 0xFFFFFFFF);
    mContext->OMSetDepthStencilState(mCommonStates->DepthDefault(), 0);
    mContext->RSSetState(mCommonStates->CullNone());
    mDebugBatch->DrawScene(view);
    mDebugBatch->DrawHollowSprite(player->Position - player->HalfSize, player->HalfSize * 2, Vector4(1, 1, 1, 1), 0, view);

    mSwapChain->Present(0, 0);
}
