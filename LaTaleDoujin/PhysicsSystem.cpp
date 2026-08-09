#include "PhysicsSystem.h"
#include <algorithm>
#include <iostream>

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

    AABB broad{
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

template <typename RandomIt, typename Compare>
bool insertion_sort_changed(RandomIt first, RandomIt last, Compare comp)
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
                if (e.Body->Type() == other->Type())
                    continue;

                Entity* entity = (Entity*)(e.Body->Type() == BodyType::Entity ? e.Body : other);
                Body* body = e.Body->Type() != BodyType::Entity ? e.Body : other;

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
        entity->CollisionTime = Vector2(1, 1);
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
                if (tmp.y >= 0 && platform->IsOneway)
                    continue;
            }
            break;
        }

        if (tmp.x) entity->CollisionTime.x = min(time, entity->CollisionTime.x);
        if (tmp.y) entity->CollisionTime.y = min(time, entity->CollisionTime.y);
    }

    // integrate positions
    for (auto& entity : m_Entities)
    {
        entity->Position += entity->Velocity * entity->CollisionTime * delta;
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

                time = SweptAABB(entity->GetAABB(), platform->GetAABB(), entity->Velocity * delta, tmp);
                entity->IsGrounded |= !time && tmp.y < 0;
                if (entity->IsGrounded) entity->Velocity = Vector2();
            }
            break;
        }
    }
}
