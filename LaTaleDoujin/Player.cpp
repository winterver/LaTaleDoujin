#include "Player.h"
#include "PhysicsSystem.h"

Player::Player(Entity* entity)
    : m_MaxJumps(2)
    , m_Speed(400)
    , m_JumpForce(600)
    , m_Entity(entity)
    , m_NumJumps(0)
    , m_WasGrounded(false)
{
}

Player::~Player() = default;

void Player::Update()
{
    if (!m_WasGrounded && m_Entity->IsGrounded)
    {
        m_NumJumps = 0;
    }
    m_WasGrounded = m_Entity->IsGrounded;
}

void Player::MoveLeft()
{
    if (m_Entity->IsGrounded)
    {
        m_Entity->Velocity.x = -m_Speed;
    }
}

void Player::MoveRight()
{
    if (m_Entity->IsGrounded)
    {
        m_Entity->Velocity.x = +m_Speed;
    }
}

void Player::Jump(bool zerox)
{
    if (m_NumJumps < m_MaxJumps)
    {
        m_Entity->Velocity.y = -m_JumpForce;
        if (zerox && m_NumJumps)
        {
            m_Entity->Velocity.x = 0;
        }
        m_NumJumps++;
    }
}

const Entity* Player::GetEntity()
{
    return m_Entity;
}
