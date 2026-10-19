#include "Player.h"
#include "PhysicsSystem.h"

Player::Player(Entity* entity)
    : mMaxJumps(2)
    , mSpeed(400)
    , mJumpForce(600)
    , mEntity(entity)
    , mNumJumps(0)
    , mWasGrounded(false)
    , mWantLeft(false)
    , mWantRight(false)
{
}

Player::~Player() = default;

void Player::Update()
{
    if (mEntity->IsGrounded)
    {
        mNumJumps = 0;
    }
    mWasGrounded = mEntity->IsGrounded;
    mWantLeft = false;
    mWantRight = false;
}

void Player::MoveLeft()
{
    if (mEntity->IsGrounded)
    {
        mEntity->Velocity.x = -mSpeed;
    }
    mWantLeft = true;
}

void Player::MoveRight()
{
    if (mEntity->IsGrounded)
    {
        mEntity->Velocity.x = +mSpeed;
    }
    mWantRight = true;
}

void Player::Jump(bool zerox)
{
    if (mNumJumps < mMaxJumps)
    {
        mEntity->Velocity.y = -mJumpForce;
        if (zerox)
        {
            if (mNumJumps)
                mEntity->Velocity.x = 0;
        }
        else
        {
            int dir = 0;
            if (mWantRight) dir = 1;
            if (mWantLeft) dir = -1;
            mEntity->Velocity.x = dir * mSpeed;
        }
        mNumJumps++;
    }
}

const Entity* Player::GetEntity()
{
    return mEntity;
}
