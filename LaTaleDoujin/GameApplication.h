#pragma once
#include "D3D11Application.h"
#include <thread>

class GameScene;

class GameApplication : public D3D11Application
{
public:
    GameApplication(const TCHAR* title, int width, int height);
    virtual ~GameApplication();

protected:
    virtual void InitScene() override;
    virtual void UpdateScene() override;
    virtual void DrawScene() override;

    void ChangeScene(unique_ptr<GameScene> scene);

private:
    unique_ptr<GameScene> mCurrentScene;
    unique_ptr<GameScene> mNextScene;
    unique_ptr<std::thread> mLoader;
    bool mLoaderFinished{};
};

