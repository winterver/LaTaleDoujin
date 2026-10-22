#include "GameScene.h"
#include <WICTextureLoader.h>

using namespace DirectX;

enum GameScene::InitStatus
{
    Uninitialized,
    Initializing,
    Initialized,
};

GameScene::GameScene(GameApplication* game)
    : mGame(game)
    , mTimer(game->GetTimer())
    , mDevice(game->GetDevice())
    , mContext(game->GetContext())
    , mSwapChain(game->GetSwapChain())
    , mRenderTargetView(game->GetRenderTargetView())
    , mDepthStencilView(game->GetDepthStencilView())
    , mStatus(Uninitialized)
{
}

GameScene::~GameScene() = default;

void GameScene::StepInit()
{
    if (mStatus == Uninitialized)
    {
        InitScene();
        mIndex = mTasks.begin();
        mStatus = Initializing;
    }

    if (mStatus == Initializing)
    {
        // Use half of interval (target frame length) to load assets
        INT64 due = mTimer->GetInterval() / 2 + mTimer->GetTime();
        while (mIndex < mTasks.end() && mTimer->GetTime() < due)
        {
            (*mIndex)();
            mIndex++;
        }

        if (mIndex == mTasks.end())
        {
            // Swap mTasks with an empty one, effectively destruct it.
            decltype(mTasks)().swap(mTasks);
            mStatus = Initialized;
        }
    }
}

bool GameScene::IsInitIncomplete()
{
    return mStatus != Initialized;
}

float GameScene::GetInitProgress()
{
    switch (mStatus)
    {
    case Uninitialized:
        return 0.0f;
    case Initializing:
        return float(mTasks.end() - mIndex) / mTasks.size();
    case Initialized:
        return 1.0f;
    }
}

void GameScene::LoadTexture(ID3D11ShaderResourceView** texture, const TCHAR* path, bool mipmap)
{
    mTasks.push_back([=] {
        ThrowIfFailed(CreateWICTextureFromFile(mDevice, mipmap ? mContext : nullptr, path, nullptr, texture));
    });
}

void GameScene::InitScene()
{
    mCommonStates = make_unique<CommonStates>(mDevice);
    mSpriteBatch = make_unique<SpriteBatch>(mContext);
}

void GameScene::UpdateScene()
{

}

void GameScene::DrawScene()
{

}
