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

struct Body {
    BodyType Type;
    Body(BodyType type) : Type(type) { }
    virtual float GetEndpointX(bool isStart) = 0;
};

struct Platform : Body
{
    Vector2 Position;
    Vector2 Size;
    bool IsOneway;

    Platform(Vector2 position, Vector2 size, bool isOneway)
        : Body(BodyType::Platform)
        , Position(position)
        , Size(size)
        , IsOneway(isOneway)
    { }

    float GetEndpointX(bool isStart)
    {
        return Position.x + isStart ? 0 : Size.x;
    }
};

struct Slope : Body
{
    Vector2 LeftEnd;
    Vector2 RightEnd;

    Slope(Vector2 leftEnd, Vector2 rightEnd)
        : Body(BodyType::Slope)
        , LeftEnd(leftEnd)
        , RightEnd(rightEnd)
    { }

    float GetEndpointX(bool isStart)
    {
        return (isStart ? LeftEnd : RightEnd).x;
    }
};

struct Entity : Body
{
    Vector2 Position;
    Vector2 Velocity;
    Vector2 HalfSize;
    Vector2 CollisionTime;
    Body* Ground;
    bool IsGrounded;

    Entity(Vector2 position, Vector2 halfSize)
        : Body(BodyType::Entity)
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
    Body* Body;
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
    std::vector<std::unique_ptr<Body>> m_Bodies;
    std::vector<std::unique_ptr<Entity>> m_Entities;
    std::vector<Endpoint> m_Endpoints;
};

