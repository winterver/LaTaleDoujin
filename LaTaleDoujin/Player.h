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
    int m_MaxJumps;
    int m_Speed;
    int m_JumpForce;

private:
    Entity* m_Entity;
    int m_NumJumps;
    bool m_WasGrounded;
};

