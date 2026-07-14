#include "World.h"

#include <algorithm>
#include <cmath>

namespace ast {

namespace {

constexpr float kTwoPi = 6.28318530718f;

// Mantem o angulo em [0, 2pi) — sem isso ele cresce sem limite numa partida
// longa e a precisao do float derrete.
float normalizeAngle(float angle)
{
    angle = std::fmod(angle, kTwoPi);
    return angle < 0.0f ? angle + kTwoPi : angle;
}

} // namespace

World::World(const WorldConfig config): m_config(config) {}

void World::setRotationAxis(const float axis)
{
    m_rotationAxis = std::clamp(axis, -1.0f, 1.0f);
}

void World::setThrust(const bool thrusting)
{
    m_thrusting = thrusting;
}

Vec2 World::shipHeading() const
{
    // 0 = para cima (-Y) e o angulo cresce no sentido horario na tela.
    return { std::sin(m_shipAngle), -std::cos(m_shipAngle) };
}

bool World::fire()
{
    if (m_shotCount >= kMaxShots || m_fireCooldown > 0.0)
    {
        return false;
    }

    const Vec2 heading = shipHeading();

    m_shotPos[m_shotCount] = m_shipPos;
    m_shotVel[m_shotCount] = { heading.x * kShotSpeed, heading.y * kShotSpeed };

    if (m_config.shotsInheritShipVelocity)
    {
        m_shotVel[m_shotCount].x += m_shipVel.x;
        m_shotVel[m_shotCount].y += m_shipVel.y;
    }

    m_shotLife[m_shotCount] = kShotLife;
    ++m_shotCount;

    m_fireCooldown = kFireCooldown;
    return true;
}

void World::update(const double dt)
{
    if (dt <= 0.0)
    {
        return;
    }

    m_fireCooldown = std::max(0.0, m_fireCooldown - dt);

    updateShip(dt);
    updateShots(dt);
}

void World::updateShip(const double dt)
{
    const auto step = static_cast<float>(dt);

    m_shipAngle = normalizeAngle(m_shipAngle + m_rotationAxis * kRotationSpeed * step);

    if (m_thrusting)
    {
        const Vec2 heading = shipHeading();
        m_shipVel.x += heading.x * kThrustAccel * step;
        m_shipVel.y += heading.y * kThrustAccel * step;
    }

    // Atrito exponencial: independente do dt (ao contrario de multiplicar por
    // uma constante por quadro), preservando a simulacao com passo fixo.
    const float damping = std::exp(-kDrag * step);
    m_shipVel.x *= damping;
    m_shipVel.y *= damping;

    const float speed = std::sqrt(m_shipVel.x * m_shipVel.x + m_shipVel.y * m_shipVel.y);
    if (speed > kMaxSpeed)
    {
        const float scale = kMaxSpeed / speed;
        m_shipVel.x *= scale;
        m_shipVel.y *= scale;
    }

    m_shipPos = wrap({ m_shipPos.x + m_shipVel.x * step, m_shipPos.y + m_shipVel.y * step });
}

void World::updateShots(const double dt)
{
    const auto step = static_cast<float>(dt);

    // Percorre de tras para frente: removeShot() troca o removido pelo ultimo.
    for (uint32_t i = m_shotCount; i > 0; --i)
    {
        const uint32_t index = i - 1;

        m_shotLife[index] -= dt;
        if (m_shotLife[index] <= 0.0)
        {
            removeShot(index);
            continue;
        }

        m_shotPos[index] = wrap({ m_shotPos[index].x + m_shotVel[index].x * step,
                                  m_shotPos[index].y + m_shotVel[index].y * step });
    }
}

void World::removeShot(const uint32_t index)
{
    const uint32_t last = m_shotCount - 1;

    m_shotPos[index] = m_shotPos[last];
    m_shotVel[index] = m_shotVel[last];
    m_shotLife[index] = m_shotLife[last];

    --m_shotCount;
}

Vec2 World::shotPosition(const uint32_t index) const
{
    return index < m_shotCount ? m_shotPos[index] : Vec2{};
}

Vec2 World::wrap(Vec2 point)
{
    if (point.x < 0.0f)
    {
        point.x += kArenaW;
    }
    else if (point.x >= kArenaW)
    {
        point.x -= kArenaW;
    }

    if (point.y < 0.0f)
    {
        point.y += kArenaH;
    }
    else if (point.y >= kArenaH)
    {
        point.y -= kArenaH;
    }

    return point;
}

} // namespace ast
