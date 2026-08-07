#include "PhysicsSystem.h"
#include <algorithm>

struct AABB
{
    float x, y;
    float w, h;
};

static bool SimpleAABB(const AABB& b1, const AABB& b2) {
    float l = b2.x - (b1.x + b1.w);
    float r = (b2.x + b2.w) - b1.x;
    float t = (b2.y + b2.h) - b1.y;
    float b = b2.y - (b1.y + b1.h);
    return !(l > 0 || r < 0 || t < 0 || b > 0);
}

static float SweptAABB(const AABB& b1, const AABB& b2, const Vector2& vel, Vector2& normal) {
    float xInvEntry, yInvEntry;
    float xInvExit, yInvExit;
    float xEntry, yEntry;
    float xExit, yExit;
    float entryTime;
    float exitTime;

    AABB broad {
        b1.x + vel.x,
        b1.y + vel.y,
        b1.w, b1.h,
    };

    if (!SimpleAABB(broad, b2)) {
        normal = {0, 0};
        return 1.0f;
    }

    xInvEntry = (vel.x > 0) ? b2.x - (b1.x + b1.w) : (b2.x + b2.w) - b1.x;
    xInvExit  = (vel.x > 0) ? (b2.x + b2.w) - b1.x : b2.x - (b1.x + b1.w);
    yInvEntry = (vel.y > 0) ? b2.y - (b1.y + b1.h) : (b2.y + b2.h) - b1.y;
    yInvExit  = (vel.y > 0) ? (b2.y + b2.h) - b1.y : b2.y - (b1.y + b1.h);

    xEntry = std::abs(vel.x) < 1e-5f ? -INFINITY : xInvEntry / vel.x;
    xExit  = std::abs(vel.x) < 1e-5f ?  INFINITY : xInvExit  / vel.x;
    yEntry = std::abs(vel.y) < 1e-5f ? -INFINITY : yInvEntry / vel.y;
    yExit  = std::abs(vel.y) < 1e-5f ?  INFINITY : yInvExit  / vel.y;

    entryTime = max(xEntry, yEntry);
    exitTime  = min(xExit, yExit);

    if (entryTime > exitTime || (xEntry < 0 && yEntry < 0) || xEntry > 1.0f || yEntry > 1.0f) {
        normal = {0, 0};
        return 1.0f;
    }

    if (xEntry > yEntry)
        normal = { xInvEntry < 0 ? 1.0f : -1.0f, 0.0f };
    else
        normal = { 0.0f, yInvEntry < 0 ? 1.0f : -1.0f };

    return entryTime;
}

PhysicsBody* PhysicsSystem::CreateBody(Vector2 position, Vector2 halfSize, int flags)
{
    auto body = std::make_unique<PhysicsBody>(PhysicsBody{ position, Vector2(), halfSize, Vector2(), Vector2(), flags});
    PhysicsBody* pbody = body.get();

    m_Bodies.push_back(std::move(body));
    m_Endpoints.push_back(Endpoint{ true, pbody });
    m_Endpoints.push_back(Endpoint{ false, pbody });

    return pbody;
}

void PhysicsSystem::Update(float delta)
{
    // integrate velocities
    for (auto& body : m_Bodies)
    {
        if (body->Flags & PHYSICS_BODY_FLAG_ENTITY)
        {
            body->Velocity += Vector2(0, 1000) * delta;
            body->CollisionTime = Vector2(1, 1);
            body->CollisionNormal = Vector2();
        }
    }

    // broadphase
    std::sort(
        m_Endpoints.begin(),
        m_Endpoints.end(),
        [](auto& a, auto& b)
        {
            return a.GetX() < b.GetX();
        }
    );

    std::vector<std::pair<PhysicsBody*, PhysicsBody*>> pairs;
    std::vector<PhysicsBody*> active;

    for (auto& e : m_Endpoints)
    {
        if (e.IsStart)
        {
            for (PhysicsBody* other : active)
            {
                if (((e.Body->Flags & PHYSICS_BODY_FLAG_PLATFORM) && (other->Flags & PHYSICS_BODY_FLAG_PLATFORM))
                    || ((e.Body->Flags & PHYSICS_BODY_FLAG_ENTITY) && (other->Flags & PHYSICS_BODY_FLAG_ENTITY)))
                    continue;

                pairs.push_back({ e.Body, other });
            }
            active.push_back(e.Body);
        }
        else
        {
            std::remove(active.begin(), active.end(), e.Body);
        }
    }

    // narrowphase, swept aabb
    for (auto& pair : pairs)
    {
        PhysicsBody* entity = pair.first->Flags & PHYSICS_BODY_FLAG_ENTITY ? pair.first : pair.second;
        PhysicsBody* platform = pair.first->Flags & PHYSICS_BODY_FLAG_PLATFORM ? pair.first : pair.second;

        AABB aabb1 = {
            (entity->Position - entity->HalfSize).x,
            (entity->Position - entity->HalfSize).y,
            entity->HalfSize.x * 2,
            entity->HalfSize.y * 2,
        };

        AABB aabb2 = {
            (platform->Position - platform->HalfSize).x,
            (platform->Position - platform->HalfSize).y,
            platform->HalfSize.x * 2,
            platform->HalfSize.y * 2,
        };

        Vector2 tmp;
        float time = SweptAABB(aabb1, aabb2, entity->Velocity * delta, tmp);

        if (tmp.y >= 0 && (platform->Flags & PHYSICS_BODY_FLAG_ONEWAY))
            continue;

        if (tmp.x && time < entity->CollisionTime.x) {
            entity->CollisionTime.x = time;
            entity->CollisionNormal.x = tmp.x;
        }

        if (tmp.y && time < entity->CollisionTime.y) {
            entity->CollisionTime.y = time;
            entity->CollisionNormal.y = tmp.y;
        }
    }

    // integrate positions
    for (auto& body : m_Bodies)
    {
        if (!(body->Flags & PHYSICS_BODY_FLAG_ENTITY))
            continue;

        if (!body->CollisionTime.x || !body->CollisionTime.y) {
            if (!body->CollisionTime.x) {
                body->Position.y += body->Velocity.y * body->CollisionTime.y * delta;
            }
            if (!body->CollisionTime.y) {
                body->Position.x += body->Velocity.x * body->CollisionTime.x * delta;
            }
        }
        else {
            body->Position.x += body->Velocity.x * body->CollisionTime.x * delta;
            body->Position.y += body->Velocity.y * body->CollisionTime.y * delta;
        }

        if (body->IsGrounded()) {
            body->Velocity = Vector2();
        }
    }
}
