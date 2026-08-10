#include "PhysicsSystem.h"
#include <algorithm>
#include <iostream>

static bool SimpleAABB(const AABB& b1, const AABB& b2) {
    float l = b2.x - (b1.x + b1.w);
    float r = (b2.x + b2.w) - b1.x;
    float t = (b2.y + b2.h) - b1.y;
    float b = b2.y - (b1.y + b1.h);
    return !(l >= 0 || r <= 0 || t <= 0 || b >= 0);
}

static float SweptAABB(const AABB& b1, const AABB& b2, const Vector2& vel, Vector2& normal) {
    float xInvEntry, yInvEntry;
    float xInvExit, yInvExit;
    float xEntry, yEntry;
    float xExit, yExit;
    float entryTime;
    float exitTime;

    AABB broad {
        vel.x > 0 ? b1.x : b1.x + vel.x,
        vel.y > 0 ? b1.y : b1.y + vel.y,
        b1.w + std::abs(vel.x),
        b1.h + std::abs(vel.y),
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

    // When distance < 1, consider it as zero.
    // So as to prevent accidental penetration caused by floating point error
    if (std::abs(xInvEntry) < 1.0f) xInvEntry = 0;
    if (std::abs(xInvExit) < 1.0f) xInvExit = 0;
    if (std::abs(yInvEntry) < 1.0f) yInvEntry = 0;
    if (std::abs(yInvExit) < 1.0f) yInvExit = 0;

    xEntry = std::abs(vel.x) < 1e-5f ? -INFINITY : xInvEntry / vel.x;
    xExit  = std::abs(vel.x) < 1e-5f ?  INFINITY : xInvExit  / vel.x;
    yEntry = std::abs(vel.y) < 1e-5f ? -INFINITY : yInvEntry / vel.y;
    yExit  = std::abs(vel.y) < 1e-5f ?  INFINITY : yInvExit  / vel.y;

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

template <typename RandomIt, typename Compare>
static bool insertion_sort_changed(RandomIt first, RandomIt last, Compare comp)
{
    bool changed = false;

    for (auto it = first + 1; it != last; ++it)
    {
        auto value = std::move(*it);
        auto hole = it;

        while (hole != first && comp(value, *(hole - 1)))
        {
            *hole = std::move(*(hole - 1));
            --hole;
            changed = true;
        }

        *hole = std::move(value);
    }

    return changed;
}

static Vector2 Reject(Vector2 V, Vector2 N)
{
    return V - V.Dot(N) * N;
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

std::vector<std::pair<Entity*, Body*>>& PhysicsSystem::BroadPhase()
{
    bool changed = insertion_sort_changed(
        m_Endpoints.begin(),
        m_Endpoints.end(),
        [](auto& a, auto& b)
        {
            return a.GetX() < b.GetX();
        }
    );

    if (!changed)
        return m_Pairs;

    std::vector<Body*> active;
    m_Pairs.clear();

    for (auto& e : m_Endpoints)
    {
        if (e.IsStart)
        {
            for (Body* other : active)
            {
                bool eIsEntity = e.Body->Type() == BodyType::Entity;
                bool otherIsEntity = other->Type() == BodyType::Entity;

                if (eIsEntity == otherIsEntity)
                    continue;

                Entity* entity = (Entity*)(eIsEntity ? e.Body : other);
                Body* body = eIsEntity ? other : e.Body;

                m_Pairs.push_back({ entity, body });
            }
            active.push_back(e.Body);
        }
        else
        {
            std::remove(active.begin(), active.end(), e.Body);
        }
    }

    return m_Pairs;
}

void PhysicsSystem::Update(float delta)
{
    // integrate velocities
    for (auto& entity : m_Entities)
    {
        entity->Velocity += Vector2(0, 1000) * delta;
        entity->CollisionTime = 1.0f;
        entity->CollisionNormal = Vector2();
        entity->IsGrounded = false;
    }

    for (auto& pair : BroadPhase())
    {
        Entity* entity = pair.first;
        float time = 1.0f;
        Vector2 tmp;

        switch (pair.second->Type())
        {
            case BodyType::Platform:
            {
                Platform* platform = (Platform*)pair.second;

                time = SweptAABB(entity->GetAABB(), platform->GetAABB(), entity->Velocity * delta, tmp);
                if (tmp.y >= 0 && platform->IsOneway) continue;

                if (time)
                    entity->CollisionTime = min(time, entity->CollisionTime);
                else
                    entity->CollisionNormal = tmp;
            }
            break;
        }
    }

    for (auto& entity : m_Entities)
    {
        if (entity->CollisionNormal.LengthSquared())
            entity->CollisionTime = 1.0f;
    }

    for (auto& pair : m_Pairs)
    {
        Entity* entity = pair.first;
        float time = 1.0f;
        Vector2 tmp;

        switch (pair.second->Type())
        {
            case BodyType::Platform:
            {
                Platform* platform = (Platform*)pair.second;
                if (!entity->CollisionNormal.LengthSquared()) continue;

                time = SweptAABB(entity->GetAABB(), platform->GetAABB(), Reject(entity->Velocity, entity->CollisionNormal) * delta, tmp);
                if (tmp.y >= 0 && platform->IsOneway) continue;

                entity->CollisionTime = min(time, entity->CollisionTime);
            }
            break;
        }
    }

    for (auto& entity : m_Entities)
    {
        entity->Position += Reject(entity->Velocity, entity->CollisionNormal) * entity->CollisionTime * delta;
    }

    // ground check
    for (auto& pair : BroadPhase())
    {
        Entity* entity = pair.first;
        float time = 1.0f;
        Vector2 tmp;

        switch (pair.second->Type())
        {
            case BodyType::Platform:
            {
                Platform* platform = (Platform*)pair.second;

                time = SweptAABB(entity->GetAABB(), platform->GetAABB(), Vector2(0, 1), tmp);
                entity->IsGrounded |= entity->Velocity.y >= 0 && !time && tmp.y < 0;
                if (entity->IsGrounded) entity->Velocity = Vector2();
            }
            break;
        }
    }
}
