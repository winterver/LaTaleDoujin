#pragma once
#include <SimpleMath.h>
#include <vector>
#include <memory>

using namespace DirectX::SimpleMath;

enum class BodyType
{
    Unknown,
    Platform,
    Slope,
    Entity,
};

struct PhysicsBody {
    BodyType Type;
    PhysicsBody(BodyType type) : Type(type) { }
    virtual float GetEndpointX(bool isStart) = 0;
};

struct Platform : PhysicsBody
{
    Vector2 Position;
    Vector2 Size;
    bool IsOneway;

    Platform(Vector2 position, Vector2 size, bool isOneway)
        : PhysicsBody(BodyType::Platform)
        , Position(position)
        , Size(size)
        , IsOneway(isOneway)
    { }

    float GetEndpointX(bool isStart)
    {
        return Position.x + isStart ? 0 : Size.x;
    }
};

struct Slope : PhysicsBody
{
    Vector2 LeftEnd;
    Vector2 RightEnd;

    Slope(Vector2 leftEnd, Vector2 rightEnd)
        : PhysicsBody(BodyType::Slope)
        , LeftEnd(leftEnd)
        , RightEnd(rightEnd)
    { }

    float GetEndpointX(bool isStart)
    {
        return (isStart ? LeftEnd : RightEnd).x;
    }
};

struct Entity : PhysicsBody
{
    Vector2 Position;
    Vector2 Velocity;
    Vector2 HalfSize;
    Vector2 CollisionTime;
    PhysicsBody* Ground;
    bool IsGrounded;

    Entity(Vector2 position, Vector2 halfSize)
        : PhysicsBody(BodyType::Entity)
        , Position(position)
        , HalfSize(halfSize)
        , Ground(nullptr)
        , IsGrounded(false)
    { }

    float GetEndpointX(bool isStart)
    {
        return Position.x - HalfSize.x * (isStart ? 1 : -1);
    }
};

struct Endpoint
{
    bool IsStart;
    PhysicsBody* Body;
    float GetX() {
        return Body->GetEndpointX(IsStart);
    }
};

class PhysicsSystem
{
public:
    Platform* CreatePlatform(Vector2 position, Vector2 size, bool isOneway = false);
    Slope* CreateSlope(Vector2 leftEnd, Vector2 rightEnd);
    Entity* CreateEntity(Vector2 position, Vector2 halfSize);
    void Update(float delta);

private:
    std::vector<std::unique_ptr<PhysicsBody>> m_Bodies;
    std::vector<std::unique_ptr<Entity>> m_Entities;
    std::vector<Endpoint> m_Endpoints;
};

