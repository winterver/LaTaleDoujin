#include "GameApplication.h"
#include "GameScene.h"
#include <tchar.h>
#include <WICTextureLoader.h>

#include "PhysicsSystem.h"
#include "Player.h"
#include "DebugBatch.h"

#include <iostream>

using namespace DirectX;
using Key = Keyboard::Keys;

class TestScene : public GameScene
{
public:
    TestScene(GameApplication* game) : GameScene(game) { }

protected:
    unique_ptr<PhysicsSystem> mPhysicsSystem;
    unique_ptr<Player> mPlayer;
    unique_ptr<DebugBatch> mDebugBatch;

    ComPtr<ID3D11ShaderResourceView> mIrisTexture;

    void InitScene() override
    {
        GameScene::InitScene();

        mPhysicsSystem = make_unique<PhysicsSystem>();
        auto ground1 = mPhysicsSystem->CreatePlatform(Vector2(0, 825), Vector2(1600, 50));
        auto ground2 = mPhysicsSystem->CreatePlatform(Vector2(800, 660), Vector2(800, 90), true);
        auto ground3 = mPhysicsSystem->CreatePlatform(Vector2(1200, 50), Vector2(50, 800));
        auto ground4 = mPhysicsSystem->CreatePlatform(Vector2(-10, 0), Vector2(10, 900));

        auto playerEntity = mPhysicsSystem->CreateEntity(Vector2(200, 450), Vector2(32, 75));
        mPlayer = make_unique<Player>(playerEntity);

        mDebugBatch = make_unique<DebugBatch>(Device());
        mDebugBatch->PutSolidRect(ground1->Position, ground1->Size);
        mDebugBatch->PutHollowRect(ground2->Position, ground2->Size, Vector4(0, 0, 1, 1));
        mDebugBatch->PutHollowRect(ground3->Position, ground3->Size);
        mDebugBatch->UpdateScene();

        ThrowIfFailed(CreateWICTextureFromFile(Device(), nullptr, _T("Resource/IRIS.PNG"), nullptr, &mIrisTexture));
    }

    void UpdateScene() override
    {
        GameScene::UpdateScene();

        mPlayer->Update();
        if (mGame->IsKeyDown(Key::Right)) { mPlayer->MoveRight(); }
        if (mGame->IsKeyDown(Key::Left)) { mPlayer->MoveLeft(); }
        if (mGame->IsKeyHit(Key::Space)) { mPlayer->Jump(mGame->IsKeyDown(Key::Down)); }

        mPhysicsSystem->Update(Timer()->DeltaTime());
    }

    void DrawScene() override
    {
        GameScene::DrawScene();

        Context()->ClearRenderTargetView(RenderTargetView(), Colors::CornflowerBlue);
        Context()->ClearDepthStencilView(DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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

        Context()->OMSetBlendState(mCommonStates->Opaque(), nullptr, 0xFFFFFFFF);
        Context()->OMSetDepthStencilState(mCommonStates->DepthDefault(), 0);
        Context()->RSSetState(mCommonStates->CullNone());
        mDebugBatch->DrawScene(view);
        mDebugBatch->DrawHollowSprite(player->Position - player->HalfSize, player->HalfSize * 2, Vector4(1, 1, 1, 1), 0, view);

        SwapChain()->Present(0, 0);
    }
};

class LaTaleDoujin : public GameApplication
{
public:
    LaTaleDoujin() : GameApplication(_T("La Tale Doujin"), 1600, 900) { }

protected:
    void InitScene() override
    {
        GameApplication::InitScene();
        ChangeScene(make_unique<TestScene>(this));
    }
};

#include <crtdbg.h>

extern "C"
{
    // Prioritize NVIDIA and AMD GPUs over others.
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

#if defined(_DEBUG)
int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
int WINAPI WinMain (
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
#endif

	LaTaleDoujin latale;

	if (!latale.Init())
		return -1;

	return latale.Run();
}