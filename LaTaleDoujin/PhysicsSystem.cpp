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

    if (!SimpleAABB(broad, b2))
    {
        normal = { 0, 0 };
        return 1.0f;
    }

    xInvEntry = (vel.x > 0) ? b2.x - (b1.x + b1.w) : (b2.x + b2.w) - b1.x;
    xInvExit  = (vel.x > 0) ? (b2.x + b2.w) - b1.x : b2.x - (b1.x + b1.w);
    yInvEntry = (vel.y > 0) ? b2.y - (b1.y + b1.h) : (b2.y + b2.h) - b1.y;
    yInvExit  = (vel.y > 0) ? (b2.y + b2.h) - b1.y : b2.y - (b1.y + b1.h);

    xEntry = !std::abs(vel.x) ? -INFINITY : xInvEntry / vel.x;
    xExit  = !std::abs(vel.x) ?  INFINITY : xInvExit  / vel.x;
    yEntry = !std::abs(vel.y) ? -INFINITY : yInvEntry / vel.y;
    yExit  = !std::abs(vel.y) ?  INFINITY : yInvExit  / vel.y;

    entryTime = max(xEntry, yEntry);
    exitTime  = min(xExit, yExit);

    if (entryTime > exitTime || (xEntry < 0 && yEntry < 0) || xEntry > 1.0f || yEntry > 1.0f)
    {
        normal = { 0, 0 };
        return 1.0f;
    }

    if (xEntry > yEntry)
        normal = { xInvEntry < 0 ? 1.0f : -1.0f, 0.0f };
    else
        normal = { 0.0f, yInvEntry < 0 ? 1.0f : -1.0f };

    return entryTime;
}

Platform* PhysicsSystem::CreatePlatform(Vector2 position, Vector2 size, bool isOneway)
{
    auto body = std::make_unique<Platform>(position, size, isOneway);
    Platform* pbody = body.get();

    m_Bodies.push_back(std::move(body));
    m_Endpoints.push_back(Endpoint{ true, pbody });
    m_Endpoints.push_back(Endpoint{ false, pbody });

    return pbody;
}

Slope* PhysicsSystem::CreateSlope(Vector2 leftEnd, Vector2 rightEnd)
{
    auto body = std::make_unique<Slope>(leftEnd, rightEnd);
    Slope* pbody = body.get();

    m_Bodies.push_back(std::move(body));
    m_Endpoints.push_back(Endpoint{ true, pbody });
    m_Endpoints.push_back(Endpoint{ false, pbody });

    return pbody;
}

Entity* PhysicsSystem::CreateEntity(Vector2 position, Vector2 halfSize)
{
    auto body = std::make_unique<Entity>(position, halfSize);
    Entity* pbody = body.get();

    m_Entities.push_back(std::move(body));
    m_Endpoints.push_back(Endpoint{ true, pbody });
    m_Endpoints.push_back(Endpoint{ false, pbody });

    return pbody;
}

void PhysicsSystem::Update(float delta)
{
    // integrate velocities
    for (auto& entity : m_Entities)
    {
        entity->Velocity += Vector2(0, 1000) * delta;
        entity->CollisionTime = Vector2(1, 1);
        entity->CollisionNormal = Vector2();
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

    std::vector<std::pair<Entity*, PhysicsBody*>> pairs;
    std::vector<PhysicsBody*> active;

    for (auto& e : m_Endpoints)
    {
        if (e.IsStart)
        {
            for (PhysicsBody* other : active)
            {
                if (((e.Body->Type != BodyType::Entity) && (other->Type != BodyType::Entity))
                    || ((e.Body->Type == BodyType::Entity) && (other->Type == BodyType::Entity)))
                    continue;

                Entity* entity = (Entity*)(e.Body->Type == BodyType::Entity ? e.Body : other);
                PhysicsBody* body = e.Body->Type != BodyType::Entity ? e.Body : other;

                pairs.push_back({ entity, body });
            }
            active.push_back(e.Body);
        }
        else
        {
            std::remove(active.begin(), active.end(), e.Body);
        }
    }

    // narrowphase
    for (auto& pair : pairs)
    {
        Entity* entity = pair.first;
        float time = 1.0f;
        Vector2 tmp;

        switch (pair.second->Type)
        {
            case BodyType::Platform:
            {
                Platform* platform = (Platform*)pair.second;

                AABB aabb1 = {
                    (entity->Position - entity->HalfSize).x,
                    (entity->Position - entity->HalfSize).y,
                    entity->HalfSize.x * 2,
                    entity->HalfSize.y * 2,
                };

                AABB aabb2 = {
                    platform->Position.x,
                    platform->Position.y,
                    platform->Size.x,
                    platform->Size.y,
                };

                time = SweptAABB(aabb1, aabb2, entity->Velocity * delta, tmp);

                if (tmp.y >= 0 && platform->IsOneway)
                    continue;
            }
            break;
        }

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
    for (auto& entity : m_Entities)
    {
        if (!entity->CollisionTime.x || !entity->CollisionTime.y) {
            if (!entity->CollisionTime.x) {
                entity->Position.y += entity->Velocity.y * entity->CollisionTime.y * delta;
            }
            if (!entity->CollisionTime.y) {
                entity->Position.x += entity->Velocity.x * entity->CollisionTime.x * delta;
            }
        }
        else {
            entity->Position.x += entity->Velocity.x * entity->CollisionTime.x * delta;
            entity->Position.y += entity->Velocity.y * entity->CollisionTime.y * delta;
        }

        if (entity->IsGrounded()) {
            entity->Velocity = Vector2();
        }
    }
}
