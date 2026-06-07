#pragma once
#include "D3D11Application.h"

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

    void UpdateScene();
    void DrawScene();
};
