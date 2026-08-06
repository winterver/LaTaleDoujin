#pragma once
#include <SimpleMath.h>
#include <vector>
#include <memory>

using namespace DirectX::SimpleMath;

#define PHYSICS_BODY_FLAG_PLATFORM  (1 << 0)
#define PHYSICS_BODY_FLAG_ENTITY    (1 << 1)
#define PHYSICS_BODY_FLAG_ONEWAY    (1 << 2)

struct PhysicsBody
{
    Vector2 Position;
    Vector2 Velocity;
    Vector2 HalfSize;
    Vector2 CollisionTime;
    Vector2 CollisionNormal;
    int Flags;

    bool IsGrounded() {
        return !CollisionTime.y && CollisionNormal.y < 0;
    }
};

struct Endpoint
{
    bool IsStart;
    PhysicsBody* Body;
    float GetX() {
        return Body->Position.x - Body->HalfSize.x * (IsStart ? 1 : -1);
    }
};

class PhysicsSystem
{
public:
    PhysicsBody* CreateBody(Vector2 position, Vector2 halfSize, int flags);
    void Update(float delta);

private:
    std::vector<std::unique_ptr<PhysicsBody>> m_Bodies;
    std::vector<Endpoint> m_Endpoints;
};

