#include "World.h"

#include <algorithm>
#include <cmath>

#include <cengine/collision2d/Intersects.hpp>

namespace ast {

namespace {

constexpr float kTwoPi = 6.28318530718f;

// Velocidade de cada tamanho de rocha: quanto menor, mais rapida (o fragmento
// e mais perigoso que a pedra que o gerou — e o que da tensao ao arcade).
constexpr float kAsteroidSpeedMin[] = { 30.0f, 50.0f, 75.0f };
constexpr float kAsteroidSpeedMax[] = { 60.0f, 95.0f, 140.0f };

// Mantem o angulo em [0, 2pi) — sem isso ele cresce sem limite numa partida
// longa e a precisao do float derrete.
float normalizeAngle(float angle)
{
    angle = std::fmod(angle, kTwoPi);
    return angle < 0.0f ? angle + kTwoPi : angle;
}

Vec2 fromPolar(const float angle, const float length)
{
    return { std::sin(angle) * length, -std::cos(angle) * length };
}

} // namespace

float World::asteroidRadius(const AsteroidSize size)
{
    constexpr float kRadius[] = { 42.0f, 22.0f, 11.0f };
    return kRadius[static_cast<size_t>(size)];
}

int World::scoreFor(const AsteroidSize size)
{
    constexpr int kScore[] = { kScoreLarge, kScoreMedium, kScoreSmall };
    return kScore[static_cast<size_t>(size)];
}

World::World(const WorldConfig config): m_config(config), m_rng(config.seed)
{
    if (m_config.spawnAsteroids)
    {
        spawnWave();
    }
}

// -----------------------------------------------------------------------------
// comandos
// -----------------------------------------------------------------------------

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
    if (m_outcome == Outcome::GameOver || m_shotCount >= kMaxShots || m_fireCooldown > 0.0)
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

// -----------------------------------------------------------------------------
// simulacao
// -----------------------------------------------------------------------------

void World::update(const double dt)
{
    if (dt <= 0.0 || m_outcome == Outcome::GameOver)
    {
        // Partida encerrada: a arena congela como estava. A cena le o outcome e
        // roteia para o gameOver — quem decide o que a derrota SIGNIFICA e o
        // fluxo do jogo, nao o World.
        return;
    }

    m_fireCooldown = std::max(0.0, m_fireCooldown - dt);
    m_invulnerability = std::max(0.0, m_invulnerability - dt);

    updateShip(dt);
    updateShots(dt);
    updateAsteroids(dt);

    collideShotsWithAsteroids();
    collideShipWithAsteroids();

    // Arena limpa: a proxima onda nasce maior. (Com as rochas desligadas pela
    // config, a arena vazia e o estado desejado — nao ha onda nenhuma.)
    if (m_config.spawnAsteroids && m_asteroidCount == 0)
    {
        ++m_wave;
        spawnWave();
    }
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

void World::updateAsteroids(const double dt)
{
    const auto step = static_cast<float>(dt);

    for (uint32_t i = 0; i < m_asteroidCount; ++i)
    {
        m_asteroidPos[i] = wrap({ m_asteroidPos[i].x + m_asteroidVel[i].x * step,
                                  m_asteroidPos[i].y + m_asteroidVel[i].y * step });
    }
}

// -----------------------------------------------------------------------------
// colisao (circulo x circulo, distancia toroidal)
// -----------------------------------------------------------------------------

void World::collideShotsWithAsteroids()
{
    for (uint32_t s = m_shotCount; s > 0; --s)
    {
        const uint32_t shot = s - 1;

        for (uint32_t a = m_asteroidCount; a > 0; --a)
        {
            const uint32_t asteroid = a - 1;

            if (!circlesOverlap(m_shotPos[shot], kShotRadius, m_asteroidPos[asteroid],
                                asteroidRadius(m_asteroidSize[asteroid])))
            {
                continue;
            }

            award(m_asteroidSize[asteroid]); // antes de partir: o tamanho e o que vale
            splitAsteroid(asteroid);
            removeShot(shot);
            break; // o tiro morreu: nao ha o que testar contra as outras rochas
        }
    }
}

void World::collideShipWithAsteroids()
{
    if (shipInvulnerable())
    {
        return;
    }

    for (uint32_t a = m_asteroidCount; a > 0; --a)
    {
        const uint32_t asteroid = a - 1;

        if (!circlesOverlap(m_shipPos, kShipRadius, m_asteroidPos[asteroid],
                            asteroidRadius(m_asteroidSize[asteroid])))
        {
            continue;
        }

        // A rocha tambem se parte na batida — senao a nave renasceria dentro
        // dela e o jogador perderia tudo de uma vez. Mas NAO pontua: seria
        // premiar a batida.
        splitAsteroid(asteroid);
        loseLife();
        return; // uma batida por quadro basta: a nave ja saiu de la
    }
}

void World::award(const AsteroidSize destroyed)
{
    m_score += scoreFor(destroyed);

    // Vida extra do arcade. `while` (e nao `if`) porque um unico tiro pode, em
    // teoria, cruzar mais de um limiar se as constantes mudarem.
    while (m_score >= m_nextExtraLife)
    {
        ++m_lives;
        m_nextExtraLife += kExtraLifeScore;
    }
}

void World::loseLife()
{
    --m_lives;

    if (m_lives <= 0)
    {
        m_lives = 0;
        m_outcome = Outcome::GameOver;
        return; // sem renascer: a nave sai da arena
    }

    respawnShip();
}

bool World::circlesOverlap(const Vec2 a, const float radiusA, const Vec2 b, const float radiusB)
{
    // O toro e nosso: reposicionamos `b` no ponto equivalente MAIS PROXIMO de
    // `a` (que pode estar do outro lado da borda)...
    const Vec2 delta = toroidalDelta(a, b);
    const Vec2 nearest = { a.x + delta.x, a.y + delta.y };

    // ...e so entao perguntamos a engine, que nao sabe do wrap e nem precisa.
    return cengine::collision2d::intersects(cengine::collision2d::Circle{ a, radiusA },
                                            cengine::collision2d::Circle{ nearest, radiusB });
}

Vec2 World::toroidalDelta(const Vec2 from, const Vec2 to)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    // Mais de meia arena de distancia? Entao o caminho curto e pela borda.
    if (dx > kArenaW * 0.5f)
    {
        dx -= kArenaW;
    }
    else if (dx < -kArenaW * 0.5f)
    {
        dx += kArenaW;
    }

    if (dy > kArenaH * 0.5f)
    {
        dy -= kArenaH;
    }
    else if (dy < -kArenaH * 0.5f)
    {
        dy += kArenaH;
    }

    return { dx, dy };
}

// -----------------------------------------------------------------------------
// rochas: onda, fragmentacao, remocao
// -----------------------------------------------------------------------------

void World::spawnWave()
{
    const uint32_t count = std::min(kAsteroidsFirstWave + m_wave - 1, kMaxAsteroidsPerWave);

    for (uint32_t i = 0; i < count; ++i)
    {
        // Sorteia longe da nave: uma rocha nascendo em cima dela seria uma
        // morte que o jogador nao teve como evitar.
        Vec2 position = {};
        do
        {
            position = { randRange(0.0f, kArenaW), randRange(0.0f, kArenaH) };
        } while (circlesOverlap(position, kSpawnSafeRadius, m_shipPos, kShipRadius));

        const float angle = randRange(0.0f, kTwoPi);
        const float speed = randRange(kAsteroidSpeedMin[0], kAsteroidSpeedMax[0]);

        addAsteroid(position, fromPolar(angle, speed), AsteroidSize::Large);
    }
}

void World::addAsteroid(const Vec2 position, const Vec2 velocity, const AsteroidSize size)
{
    if (m_asteroidCount >= kMaxAsteroids)
    {
        return;
    }

    m_asteroidPos[m_asteroidCount] = position;
    m_asteroidVel[m_asteroidCount] = velocity;
    m_asteroidSize[m_asteroidCount] = size;
    ++m_asteroidCount;
}

void World::splitAsteroid(const uint32_t index)
{
    const AsteroidSize size = m_asteroidSize[index];
    const Vec2         position = m_asteroidPos[index];

    removeAsteroid(index);

    if (size == AsteroidSize::Small)
    {
        return; // a menor some de vez
    }

    const auto next = static_cast<AsteroidSize>(static_cast<uint8_t>(size) + 1);
    const auto tier = static_cast<size_t>(next);

    // Os dois fragmentos saem em direcoes sorteadas e mais rapidos que a mae.
    for (uint32_t i = 0; i < 2; ++i)
    {
        const float angle = randRange(0.0f, kTwoPi);
        const float speed = randRange(kAsteroidSpeedMin[tier], kAsteroidSpeedMax[tier]);

        addAsteroid(position, fromPolar(angle, speed), next);
    }
}

void World::removeAsteroid(const uint32_t index)
{
    const uint32_t last = m_asteroidCount - 1;

    m_asteroidPos[index] = m_asteroidPos[last];
    m_asteroidVel[index] = m_asteroidVel[last];
    m_asteroidSize[index] = m_asteroidSize[last];

    --m_asteroidCount;
}

void World::removeShot(const uint32_t index)
{
    const uint32_t last = m_shotCount - 1;

    m_shotPos[index] = m_shotPos[last];
    m_shotVel[index] = m_shotVel[last];
    m_shotLife[index] = m_shotLife[last];

    --m_shotCount;
}

void World::respawnShip()
{
    m_shipPos = { kArenaW * 0.5f, kArenaH * 0.5f };
    m_shipVel = {};
    m_shipAngle = 0.0f;
    m_invulnerability = kSpawnInvulnerability;
}

// -----------------------------------------------------------------------------
// consultas e utilitarios
// -----------------------------------------------------------------------------

Vec2 World::shotPosition(const uint32_t index) const
{
    return index < m_shotCount ? m_shotPos[index] : Vec2{};
}

Vec2 World::asteroidPosition(const uint32_t index) const
{
    return index < m_asteroidCount ? m_asteroidPos[index] : Vec2{};
}

AsteroidSize World::asteroidSize(const uint32_t index) const
{
    return index < m_asteroidCount ? m_asteroidSize[index] : AsteroidSize::Large;
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

float World::rand01()
{
    // LCG de Numerical Recipes: barato e suficiente para sortear rochas.
    m_rng = m_rng * 1664525u + 1013904223u;
    return static_cast<float>(m_rng >> 8) / static_cast<float>(1u << 24);
}

float World::randRange(const float min, const float max)
{
    return min + rand01() * (max - min);
}

} // namespace ast
