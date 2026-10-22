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
        if (mNextScene->IsInitIncomplete())
        {
            mNextScene->StepInit();
        }
        else
        {
            mCurrentScene = std::move(mNextScene);
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
