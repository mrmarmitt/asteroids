#include <gtest/gtest.h>

#include <cmath>

#include "asteroids/game/World.h"

// A simulacao da nave (task 02). Os testes dirigem o World do mesmo jeito que
// a cena: comandos -> update(dt fixo) -> consultas. Nada de plataforma aqui.

namespace {

constexpr double kStep = 1.0 / 60.0; // o passo fixo do engine

// Avanca a simulacao com o passo fixo, como o loop da cengine faria.
void advance(ast::World& world, const double seconds)
{
    for (double elapsed = 0.0; elapsed < seconds; elapsed += kStep)
    {
        world.update(kStep);
    }
}

float speedOf(const ast::Vec2 v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

} // namespace

TEST(WorldTest, ShipStartsCenteredStoppedAndPointingUp)
{
    const ast::World world;

    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
    EXPECT_FLOAT_EQ(world.shipAngle(), 0.0f);
    EXPECT_FLOAT_EQ(speedOf(world.shipVelocity()), 0.0f);

    // Angulo 0 = proa para cima: -Y na convencao da arena.
    EXPECT_NEAR(world.shipHeading().x, 0.0f, 1e-5f);
    EXPECT_NEAR(world.shipHeading().y, -1.0f, 1e-5f);
}

TEST(WorldTest, ShipDoesNotDriftWithoutThrust)
{
    ast::World world;

    advance(world, 2.0);

    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
}

TEST(WorldTest, RotationAxisTurnsTheShipInBothDirections)
{
    ast::World world;

    // Um quarto de volta para a direita (horario na tela): proa aponta para +X.
    world.setRotationAxis(1.0f);
    advance(world, 1.5707963f / ast::World::kRotationSpeed);

    EXPECT_NEAR(world.shipHeading().x, 1.0f, 0.05f);
    EXPECT_NEAR(world.shipHeading().y, 0.0f, 0.05f);

    // De volta para a esquerda: proa aponta para cima outra vez.
    world.setRotationAxis(-1.0f);
    advance(world, 1.5707963f / ast::World::kRotationSpeed);

    EXPECT_NEAR(world.shipHeading().x, 0.0f, 0.05f);
    EXPECT_NEAR(world.shipHeading().y, -1.0f, 0.05f);
}

TEST(WorldTest, AngleStaysNormalizedAfterManyTurns)
{
    ast::World world;

    world.setRotationAxis(1.0f);
    advance(world, 20.0); // varias voltas completas

    EXPECT_GE(world.shipAngle(), 0.0f);
    EXPECT_LT(world.shipAngle(), 6.2831854f);
}

TEST(WorldTest, ThrustAcceleratesAlongTheHeading)
{
    ast::World world;

    world.setThrust(true);
    advance(world, 0.5);

    // Proa para cima: a nave sobe (Y diminui) e nao deriva no eixo X.
    EXPECT_LT(world.shipVelocity().y, 0.0f);
    EXPECT_NEAR(world.shipVelocity().x, 0.0f, 1e-3f);
    EXPECT_LT(world.shipPosition().y, ast::World::kArenaH * 0.5f);
    EXPECT_NEAR(world.shipPosition().x, ast::World::kArenaW * 0.5f, 1e-3f);
}

TEST(WorldTest, ShipKeepsMovingAfterThrustReleasedButDragSlowsItDown)
{
    ast::World world;

    world.setThrust(true);
    advance(world, 0.5);

    const float launchSpeed = speedOf(world.shipVelocity());
    ASSERT_GT(launchSpeed, 0.0f);

    world.setThrust(false);
    const ast::Vec2 released = world.shipPosition();
    advance(world, 1.0);

    // Inercia: continua andando sem motor...
    EXPECT_NE(world.shipPosition().y, released.y);
    // ...mas o atrito come a velocidade (senao a nave fica ingovernavel).
    EXPECT_LT(speedOf(world.shipVelocity()), launchSpeed);
    EXPECT_GT(speedOf(world.shipVelocity()), 0.0f);
}

TEST(WorldTest, SpeedSaturatesAtMaxSpeed)
{
    ast::World world;

    world.setThrust(true);
    advance(world, 30.0); // muito alem do tempo necessario para saturar

    EXPECT_LE(speedOf(world.shipVelocity()), ast::World::kMaxSpeed + 1e-3f);
}

TEST(WorldTest, ShipWrapsAroundTheArenaEdges)
{
    ast::World world;

    // Sobe (proa para cima) ate cruzar a borda de cima: reaparece embaixo.
    world.setThrust(true);
    advance(world, 4.0);

    EXPECT_GE(world.shipPosition().y, 0.0f);
    EXPECT_LT(world.shipPosition().y, ast::World::kArenaH);
    EXPECT_GT(world.shipPosition().y, ast::World::kArenaH * 0.5f) << "deveria ter dado a volta pela borda de cima";
}

TEST(WorldTest, FireSpawnsShotAtTheShipMovingForward)
{
    ast::World world{ ast::WorldConfig{ .shotsInheritShipVelocity = false } };

    ASSERT_TRUE(world.fire());
    ASSERT_EQ(world.shotCount(), 1u);
    EXPECT_FLOAT_EQ(world.shotPosition(0).x, world.shipPosition().x);
    EXPECT_FLOAT_EQ(world.shotPosition(0).y, world.shipPosition().y);

    const ast::Vec2 spawn = world.shotPosition(0);
    world.update(kStep);

    // Proa para cima: o tiro sobe.
    EXPECT_LT(world.shotPosition(0).y, spawn.y);
    EXPECT_FLOAT_EQ(world.shotPosition(0).x, spawn.x);
}

TEST(WorldTest, FireRespectsCooldown)
{
    ast::World world;

    EXPECT_TRUE(world.fire());
    EXPECT_FALSE(world.fire()) << "segundo tiro no mesmo quadro deveria bater no cooldown";

    advance(world, ast::World::kFireCooldown + kStep);
    EXPECT_TRUE(world.fire());
    EXPECT_EQ(world.shotCount(), 2u);
}

TEST(WorldTest, FireIsCappedAtMaxShotsInFlight)
{
    ast::World world;

    for (uint32_t i = 0; i < ast::World::kMaxShots; ++i)
    {
        ASSERT_TRUE(world.fire()) << "tiro " << i;
        advance(world, ast::World::kFireCooldown + kStep);
    }

    ASSERT_EQ(world.shotCount(), ast::World::kMaxShots);
    EXPECT_FALSE(world.fire()) << "cheio: nenhum tiro a mais entra em voo";
}

TEST(WorldTest, ShotsExpireAfterTheirLifetime)
{
    ast::World world;

    ASSERT_TRUE(world.fire());
    ASSERT_EQ(world.shotCount(), 1u);

    advance(world, ast::World::kShotLife + kStep);

    EXPECT_EQ(world.shotCount(), 0u);
}

TEST(WorldTest, ShotsWrapAroundTheArenaEdges)
{
    ast::World world{ ast::WorldConfig{ .shotsInheritShipVelocity = false } };

    ASSERT_TRUE(world.fire()); // sobe a partir do centro

    // Tempo suficiente para o tiro cruzar a borda de cima (300 de arena a
    // kShotSpeed), mas dentro do tempo de vida dele.
    advance(world, 0.8);

    ASSERT_EQ(world.shotCount(), 1u);
    EXPECT_GE(world.shotPosition(0).y, 0.0f);
    EXPECT_LT(world.shotPosition(0).y, ast::World::kArenaH);
    EXPECT_GT(world.shotPosition(0).y, ast::World::kArenaH * 0.5f) << "o tiro deveria ter dado a volta";
}

TEST(WorldTest, ShotsInheritShipVelocityWhenConfigured)
{
    ast::World fast;   // herda (padrao)
    ast::World steady{ ast::WorldConfig{ .shotsInheritShipVelocity = false } };

    for (ast::World* world : { &fast, &steady })
    {
        world->setThrust(true);
        advance(*world, 0.5);
        world->setThrust(false);
        ASSERT_TRUE(world->fire());
    }

    const float travelFast = fast.shipPosition().y - fast.shotPosition(0).y;
    const float travelSteady = steady.shipPosition().y - steady.shotPosition(0).y;
    EXPECT_FLOAT_EQ(travelFast, 0.0f);
    EXPECT_FLOAT_EQ(travelSteady, 0.0f);

    fast.update(kStep);
    steady.update(kStep);

    // Nave subindo: o tiro que herda a velocidade dela sobe MAIS no mesmo dt.
    EXPECT_LT(fast.shotPosition(0).y, steady.shotPosition(0).y);
}

TEST(WorldTest, RotationAxisIsClamped)
{
    ast::World clamped;
    ast::World normal;

    clamped.setRotationAxis(50.0f); // input maluco de uma cena
    normal.setRotationAxis(1.0f);

    advance(clamped, 0.25);
    advance(normal, 0.25);

    EXPECT_FLOAT_EQ(clamped.shipAngle(), normal.shipAngle());
}

TEST(WorldTest, NonPositiveDtDoesNothing)
{
    ast::World world;

    world.setThrust(true);
    world.update(0.0);
    world.update(-1.0);

    EXPECT_FLOAT_EQ(speedOf(world.shipVelocity()), 0.0f);
    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
}
