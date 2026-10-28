#include "GameApplication.h"
#include "GameScene.h"

GameApplication::GameApplication(const TCHAR* title, int width, int height)
    : D3D11Application(title, width, height)
{
}

GameApplication::~GameApplication() = default;

void GameApplication::InitScene()
{
    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
}

void GameApplication::UpdateScene()
{
    if (mNextScene)
    {
        if (!mLoader)
        {
            (mLoader = make_unique<std::thread>([&] {
                mNextScene->InitScene();
                mLoaderFinished = true;
            }))->detach();
        }
        else if (mLoaderFinished)
        {
            mCurrentScene = std::move(mNextScene);
            mLoaderFinished = false;
			mLoader.reset();
        }
    }

    if (mCurrentScene)
        mCurrentScene->UpdateScene();
}

void GameApplication::DrawScene()
{
    if (mCurrentScene)
        mCurrentScene->DrawScene();
}

void GameApplication::ChangeScene(unique_ptr<GameScene> scene)
{
    mNextScene = std::move(scene);
}
