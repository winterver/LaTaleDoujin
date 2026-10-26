#include "GameScene.h"

using namespace DirectX;

GameScene::GameScene(GameApplication* game)
    : mGame(game)
{
}

GameScene::~GameScene() = default;

void GameScene::InitScene()
{
    mCommonStates = make_unique<CommonStates>(Device());
    mSpriteBatch = make_unique<SpriteBatch>(Context());
}

void GameScene::UpdateScene()
{

}

void GameScene::DrawScene()
{

}
