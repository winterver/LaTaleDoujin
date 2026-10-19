#pragma once

struct Entity;

class Player
{
public:
    Player(Entity* entity);
    ~Player();

    void Update();
    void MoveLeft();
    void MoveRight();
    void Jump(bool zerox);
    const Entity* GetEntity();

public:
    int mMaxJumps;
    int mSpeed;
    int mJumpForce;

private:
    Entity* mEntity;
    int mNumJumps;
    bool mWasGrounded;
    bool mWantLeft;
    bool mWantRight;
};

