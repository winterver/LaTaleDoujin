#include "Player.h"
#include "PhysicsSystem.h"

Player::Player(Entity* entity)
    : m_MaxJumps(2)
    , m_Speed(400)
    , m_JumpForce(600)
    , m_Entity(entity)
    , m_NumJumps(0)
    , m_WasGrounded(false)
    , m_WantLeft(false)
    , m_WantRight(false)
{
}

Player::~Player() = default;

void Player::Update()
{
    if (m_Entity->IsGrounded)
    {
        m_NumJumps = 0;
    }
    m_WasGrounded = m_Entity->IsGrounded;
    m_WantLeft = false;
    m_WantRight = false;
}

void Player::MoveLeft()
{
    if (m_Entity->IsGrounded)
    {
        m_Entity->Velocity.x = -m_Speed;
    }
    m_WantLeft = true;
}

void Player::MoveRight()
{
    if (m_Entity->IsGrounded)
    {
        m_Entity->Velocity.x = +m_Speed;
    }
    m_WantRight = true;
}

void Player::Jump(bool zerox)
{
    if (m_NumJumps < m_MaxJumps)
    {
        m_Entity->Velocity.y = -m_JumpForce;
        if (zerox)
        {
            if (m_NumJumps)
                m_Entity->Velocity.x = 0;
        }
        else
        {
            int dir = 0;
            if (m_WantRight) dir = 1;
            if (m_WantLeft) dir = -1;
            m_Entity->Velocity.x = dir * m_Speed;
        }
        m_NumJumps++;
    }
}

const Entity* Player::GetEntity()
{
    return m_Entity;
}
