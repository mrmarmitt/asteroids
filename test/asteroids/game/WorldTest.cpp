#include <gtest/gtest.h>

#include <cmath>

#include "asteroids/game/World.h"

// A simulacao da partida. Os testes dirigem o World do mesmo jeito que a cena:
// comandos -> update(dt fixo) -> consultas. Nada de plataforma aqui.

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

// Arena vazia: os testes da NAVE (task 02) nao querem rochas se metendo no
// caminho — nem batendo nela, nem sendo atingidas pelos tiros dela.
ast::World shipOnlyWorld(const bool shotsInheritShipVelocity = true)
{
    return ast::World{ ast::WorldConfig{ .shotsInheritShipVelocity = shotsInheritShipVelocity,
                                         .spawnAsteroids = false } };
}

uint32_t countOfSize(const ast::World& world, const ast::AsteroidSize size)
{
    uint32_t total = 0;
    for (uint32_t i = 0; i < world.asteroidCount(); ++i)
    {
        if (world.asteroidSize(i) == size)
        {
            ++total;
        }
    }
    return total;
}

} // namespace

// =============================================================================
// A nave (task 02)
// =============================================================================

TEST(WorldTest, ShipStartsCenteredStoppedAndPointingUp)
{
    const ast::World world = shipOnlyWorld();

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
    ast::World world = shipOnlyWorld();

    advance(world, 2.0);

    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
}

TEST(WorldTest, RotationAxisTurnsTheShipInBothDirections)
{
    ast::World world = shipOnlyWorld();

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
    ast::World world = shipOnlyWorld();

    world.setRotationAxis(1.0f);
    advance(world, 20.0); // varias voltas completas

    EXPECT_GE(world.shipAngle(), 0.0f);
    EXPECT_LT(world.shipAngle(), 6.2831854f);
}

TEST(WorldTest, ThrustAcceleratesAlongTheHeading)
{
    ast::World world = shipOnlyWorld();

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
    ast::World world = shipOnlyWorld();

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
    ast::World world = shipOnlyWorld();

    world.setThrust(true);
    advance(world, 30.0); // muito alem do tempo necessario para saturar

    EXPECT_LE(speedOf(world.shipVelocity()), ast::World::kMaxSpeed + 1e-3f);
}

TEST(WorldTest, ShipWrapsAroundTheArenaEdges)
{
    ast::World world = shipOnlyWorld();

    // Sobe (proa para cima) ate cruzar a borda de cima: reaparece embaixo.
    world.setThrust(true);
    advance(world, 4.0);

    EXPECT_GE(world.shipPosition().y, 0.0f);
    EXPECT_LT(world.shipPosition().y, ast::World::kArenaH);
    EXPECT_GT(world.shipPosition().y, ast::World::kArenaH * 0.5f) << "deveria ter dado a volta pela borda de cima";
}

TEST(WorldTest, FireSpawnsShotAtTheShipMovingForward)
{
    ast::World world = shipOnlyWorld(false);

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
    ast::World world = shipOnlyWorld();

    EXPECT_TRUE(world.fire());
    EXPECT_FALSE(world.fire()) << "segundo tiro no mesmo quadro deveria bater no cooldown";

    advance(world, ast::World::kFireCooldown + kStep);
    EXPECT_TRUE(world.fire());
    EXPECT_EQ(world.shotCount(), 2u);
}

TEST(WorldTest, FireIsCappedAtMaxShotsInFlight)
{
    ast::World world = shipOnlyWorld();

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
    ast::World world = shipOnlyWorld();

    ASSERT_TRUE(world.fire());
    ASSERT_EQ(world.shotCount(), 1u);

    advance(world, ast::World::kShotLife + kStep);

    EXPECT_EQ(world.shotCount(), 0u);
}

TEST(WorldTest, ShotsWrapAroundTheArenaEdges)
{
    ast::World world = shipOnlyWorld(false);

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
    ast::World fast = shipOnlyWorld(true);
    ast::World steady = shipOnlyWorld(false);

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
    ast::World clamped = shipOnlyWorld();
    ast::World normal = shipOnlyWorld();

    clamped.setRotationAxis(50.0f); // input maluco de uma cena
    normal.setRotationAxis(1.0f);

    advance(clamped, 0.25);
    advance(normal, 0.25);

    EXPECT_FLOAT_EQ(clamped.shipAngle(), normal.shipAngle());
}

TEST(WorldTest, NonPositiveDtDoesNothing)
{
    ast::World world = shipOnlyWorld();

    world.setThrust(true);
    world.update(0.0);
    world.update(-1.0);

    EXPECT_FLOAT_EQ(speedOf(world.shipVelocity()), 0.0f);
    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
}

// =============================================================================
// As rochas (task 03)
// =============================================================================

TEST(WorldTest, FirstWaveSpawnsLargeAsteroidsAwayFromTheShip)
{
    const ast::World world;

    EXPECT_EQ(world.wave(), 1u);
    ASSERT_EQ(world.asteroidCount(), ast::World::kAsteroidsFirstWave);
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Large), ast::World::kAsteroidsFirstWave);

    // Nenhuma rocha nasce em cima da nave — seria uma morte inevitavel.
    for (uint32_t i = 0; i < world.asteroidCount(); ++i)
    {
        const ast::Vec2 rock = world.asteroidPosition(i);
        const float     dx = rock.x - world.shipPosition().x;
        const float     dy = rock.y - world.shipPosition().y;

        EXPECT_GT(std::sqrt(dx * dx + dy * dy), ast::World::kSpawnSafeRadius - ast::World::kShipRadius);
    }
}

TEST(WorldTest, SameSeedGivesTheSameWave)
{
    const ast::World a{ ast::WorldConfig{ .seed = 42 } };
    const ast::World b{ ast::WorldConfig{ .seed = 42 } };

    ASSERT_EQ(a.asteroidCount(), b.asteroidCount());
    for (uint32_t i = 0; i < a.asteroidCount(); ++i)
    {
        EXPECT_FLOAT_EQ(a.asteroidPosition(i).x, b.asteroidPosition(i).x);
        EXPECT_FLOAT_EQ(a.asteroidPosition(i).y, b.asteroidPosition(i).y);
    }
}

TEST(WorldTest, AsteroidsDriftAndStayInsideTheArena)
{
    ast::World world;

    const ast::Vec2 before = world.asteroidPosition(0);
    advance(world, 0.5);
    const ast::Vec2 after = world.asteroidPosition(0);

    EXPECT_TRUE(before.x != after.x || before.y != after.y) << "a rocha deveria estar em movimento";

    // Muito tempo depois, todas continuam DENTRO da arena (deram a volta).
    advance(world, 30.0);
    for (uint32_t i = 0; i < world.asteroidCount(); ++i)
    {
        EXPECT_GE(world.asteroidPosition(i).x, 0.0f);
        EXPECT_LT(world.asteroidPosition(i).x, ast::World::kArenaW);
        EXPECT_GE(world.asteroidPosition(i).y, 0.0f);
        EXPECT_LT(world.asteroidPosition(i).y, ast::World::kArenaH);
    }
}

// --- a geometria: circulo x circulo com distancia TOROIDAL ---

TEST(WorldTest, CirclesOverlapAcrossOppositeEdges)
{
    // Colados na borda esquerda e na direita: 10 de distancia PELA BORDA, e
    // nao 790 como uma conta euclidiana ingenua diria. Na arena-toro eles se
    // tocam — e e isso que faz o tiro que deu a volta acertar a rocha.
    EXPECT_TRUE(ast::World::circlesOverlap({ 5.0f, 300.0f }, 10.0f, { 795.0f, 300.0f }, 10.0f));

    // O mesmo no eixo Y (topo x fundo).
    EXPECT_TRUE(ast::World::circlesOverlap({ 400.0f, 3.0f }, 10.0f, { 400.0f, 597.0f }, 10.0f));
}

TEST(WorldTest, CirclesDoNotOverlapWhenGenuinelyFarApart)
{
    // Longe pelos dois caminhos (direto e pela borda): nao ha colisao.
    EXPECT_FALSE(ast::World::circlesOverlap({ 5.0f, 300.0f }, 10.0f, { 400.0f, 300.0f }, 10.0f));

    // Encostados, mas sem se tocar (raios somam 20, distancia 21).
    EXPECT_FALSE(ast::World::circlesOverlap({ 100.0f, 100.0f }, 10.0f, { 121.0f, 100.0f }, 10.0f));
}

TEST(WorldTest, ToroidalDeltaTakesTheShortWayThroughTheBorder)
{
    const ast::Vec2 delta = ast::World::toroidalDelta({ 790.0f, 300.0f }, { 10.0f, 300.0f });

    EXPECT_FLOAT_EQ(delta.x, 20.0f) << "o caminho curto atravessa a borda direita";
    EXPECT_FLOAT_EQ(delta.y, 0.0f);
}

// --- fragmentacao: arena montada pelo teste, sem depender do sorteio ---

namespace {

// Uma arena com UMA rocha parada bem na mira da nave (que nasce no centro,
// apontando para cima). Assim o tiro acerta em ~0.2s, sem perseguicao.
ast::World worldWithTargetAhead(const ast::AsteroidSize size)
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    world.addAsteroid({ ast::World::kArenaW * 0.5f, ast::World::kArenaH * 0.5f - 120.0f }, {}, size);
    return world;
}

// Avanca ate o tiro sumir (acertou ou expirou).
void advanceUntilShotGone(ast::World& world)
{
    for (int i = 0; i < 200 && world.shotCount() > 0; ++i)
    {
        world.update(kStep);
    }
}

} // namespace

TEST(WorldTest, ShotSplitsLargeAsteroidIntoTwoMediums)
{
    ast::World world = worldWithTargetAhead(ast::AsteroidSize::Large);

    ASSERT_TRUE(world.fire());
    advanceUntilShotGone(world);

    EXPECT_EQ(world.asteroidCount(), 2u);
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Medium), 2u);
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Large), 0u);
}

TEST(WorldTest, ShotSplitsMediumAsteroidIntoTwoSmalls)
{
    ast::World world = worldWithTargetAhead(ast::AsteroidSize::Medium);

    ASSERT_TRUE(world.fire());
    advanceUntilShotGone(world);

    EXPECT_EQ(world.asteroidCount(), 2u);
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Small), 2u);
}

TEST(WorldTest, SmallAsteroidIsDestroyedInsteadOfSplitting)
{
    ast::World world = worldWithTargetAhead(ast::AsteroidSize::Small);

    ASSERT_TRUE(world.fire());
    advanceUntilShotGone(world);

    EXPECT_EQ(world.asteroidCount(), 0u) << "a menor some de vez, nao se parte";
}

TEST(WorldTest, ShotIsConsumedByTheAsteroidItHits)
{
    ast::World world = worldWithTargetAhead(ast::AsteroidSize::Large);

    ASSERT_TRUE(world.fire());

    // Roda so ate o impacto: o tiro sumiu porque ACERTOU, nao porque expirou.
    for (int i = 0; i < 200 && world.asteroidCount() == 1; ++i)
    {
        world.update(kStep);
    }

    ASSERT_EQ(world.asteroidCount(), 2u) << "a rocha deveria ter se partido";
    EXPECT_EQ(world.shotCount(), 0u) << "o tiro morre no impacto";
}

TEST(WorldTest, FragmentsAreBornWhereTheParentDied)
{
    ast::World world = worldWithTargetAhead(ast::AsteroidSize::Large);

    const ast::Vec2 parent = world.asteroidPosition(0);

    ASSERT_TRUE(world.fire());
    for (int i = 0; i < 200 && world.asteroidCount() == 1; ++i)
    {
        world.update(kStep);
    }

    ASSERT_EQ(world.asteroidCount(), 2u);
    for (uint32_t i = 0; i < 2; ++i)
    {
        const ast::Vec2 fragment = world.asteroidPosition(i);
        const ast::Vec2 delta = ast::World::toroidalDelta(parent, fragment);

        // Nasceram na mae e ja andaram um quadro — nao mais que isso.
        EXPECT_LT(std::sqrt(delta.x * delta.x + delta.y * delta.y), 10.0f);
    }
}

// --- ondas ---

TEST(WorldTest, ClearingTheArenaSpawnsABiggerNextWave)
{
    ast::World world;

    ASSERT_EQ(world.wave(), 1u);
    ASSERT_EQ(world.asteroidCount(), ast::World::kAsteroidsFirstWave);

    world.clearAsteroids();
    world.update(kStep);

    EXPECT_EQ(world.wave(), 2u);
    EXPECT_EQ(world.asteroidCount(), ast::World::kAsteroidsFirstWave + 1) << "cada onda traz uma rocha a mais";
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Large), ast::World::kAsteroidsFirstWave + 1)
        << "a onda nova nasce so de rochas grandes";
}

TEST(WorldTest, WaveSizeIsCapped)
{
    ast::World world;

    // Limpa a arena varias vezes: a onda cresce, mas nao passa do teto.
    for (uint32_t i = 0; i < 20; ++i)
    {
        world.clearAsteroids();
        world.update(kStep);

        EXPECT_LE(world.asteroidCount(), ast::World::kMaxAsteroidsPerWave);
    }

    EXPECT_EQ(world.asteroidCount(), ast::World::kMaxAsteroidsPerWave);
}

// --- nave x rocha ---

TEST(WorldTest, ShipStartsInvulnerableAndBecomesVulnerable)
{
    ast::World world = shipOnlyWorld();

    EXPECT_TRUE(world.shipInvulnerable()) << "a nave nasce protegida";

    advance(world, ast::World::kSpawnInvulnerability + kStep);

    EXPECT_FALSE(world.shipInvulnerable());
}

TEST(WorldTest, InvulnerableShipIgnoresAsteroids)
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    // Rocha EM CIMA da nave, enquanto a protecao inicial dura.
    world.addAsteroid(world.shipPosition(), {}, ast::AsteroidSize::Large);
    ASSERT_TRUE(world.shipInvulnerable());

    advance(world, ast::World::kSpawnInvulnerability - 2 * kStep);

    EXPECT_EQ(world.lives(), ast::World::kInitialLives) << "protegida: a rocha atravessa a nave";
    EXPECT_EQ(world.asteroidCount(), 1u) << "e a rocha continua inteira";
}

namespace {

// Uma arena onde a nave, acelerando, bate numa rocha parada logo a frente.
// Devolve o World ja com a protecao inicial vencida.
ast::World worldWithRockInTheShipsPath()
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    advance(world, ast::World::kSpawnInvulnerability + kStep);
    world.addAsteroid({ ast::World::kArenaW * 0.5f, ast::World::kArenaH * 0.5f - 100.0f }, {},
                      ast::AsteroidSize::Large);
    world.setThrust(true);
    return world;
}

// Roda ate a nave perder uma vida (ou desistir).
void advanceUntilLifeLost(ast::World& world, const int livesBefore)
{
    for (int i = 0; i < 400 && world.lives() == livesBefore; ++i)
    {
        world.update(kStep);
    }
}

} // namespace

TEST(WorldTest, AsteroidHittingTheShipRespawnsItAtTheCenter)
{
    ast::World world = worldWithRockInTheShipsPath();

    advanceUntilLifeLost(world, ast::World::kInitialLives);

    ASSERT_EQ(world.lives(), ast::World::kInitialLives - 1) << "a nave deveria ter batido na rocha";

    // Batida: volta ao centro, parada, apontando para cima e protegida.
    EXPECT_FLOAT_EQ(world.shipPosition().x, ast::World::kArenaW * 0.5f);
    EXPECT_FLOAT_EQ(world.shipPosition().y, ast::World::kArenaH * 0.5f);
    EXPECT_FLOAT_EQ(speedOf(world.shipVelocity()), 0.0f);
    EXPECT_FLOAT_EQ(world.shipAngle(), 0.0f);
    EXPECT_TRUE(world.shipInvulnerable());
    EXPECT_TRUE(world.shipAlive());

    // A rocha tambem se parte na batida — senao a nave renasceria e a rocha
    // grande continuaria vindo em cima dela.
    EXPECT_EQ(countOfSize(world, ast::AsteroidSize::Medium), 2u);
}

// =============================================================================
// Placar da partida: pontos, vidas e gameOver (task 04)
// =============================================================================

TEST(WorldTest, GameStartsWithFullLivesAndNoScore)
{
    const ast::World world;

    EXPECT_EQ(world.lives(), ast::World::kInitialLives);
    EXPECT_EQ(world.score(), 0);
    EXPECT_EQ(world.outcome(), ast::World::Outcome::Playing);
    EXPECT_TRUE(world.shipAlive());
}

TEST(WorldTest, ShootingARockScoresByItsSize)
{
    // Quanto menor a rocha, mais dificil de acertar e mais ela vale.
    struct Case
    {
        ast::AsteroidSize size;
        int               points;
    };

    for (const auto& [size, points] : { Case{ ast::AsteroidSize::Large, ast::World::kScoreLarge },
                                        Case{ ast::AsteroidSize::Medium, ast::World::kScoreMedium },
                                        Case{ ast::AsteroidSize::Small, ast::World::kScoreSmall } })
    {
        ast::World world = worldWithTargetAhead(size);

        ASSERT_TRUE(world.fire());
        advanceUntilShotGone(world);

        EXPECT_EQ(world.score(), points) << "tamanho " << static_cast<int>(size);
    }
}

TEST(WorldTest, RammingARockDoesNotScore)
{
    ast::World world = worldWithRockInTheShipsPath();

    advanceUntilLifeLost(world, ast::World::kInitialLives);

    ASSERT_EQ(world.lives(), ast::World::kInitialLives - 1);
    EXPECT_EQ(world.score(), 0) << "bater na rocha a parte, mas premiar a batida seria absurdo";
}

TEST(WorldTest, RunningOutOfLivesEndsTheGame)
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    for (int life = ast::World::kInitialLives; life > 0; --life)
    {
        ASSERT_EQ(world.outcome(), ast::World::Outcome::Playing) << "ainda restam " << life << " vidas";

        // Espera a protecao do (re)nascimento passar e joga uma rocha em cima.
        advance(world, ast::World::kSpawnInvulnerability + kStep);
        world.addAsteroid(world.shipPosition(), {}, ast::AsteroidSize::Small);

        advanceUntilLifeLost(world, life);
        ASSERT_EQ(world.lives(), life - 1);
    }

    EXPECT_EQ(world.outcome(), ast::World::Outcome::GameOver);
    EXPECT_EQ(world.lives(), 0);
    EXPECT_FALSE(world.shipAlive()) << "sem vidas, a nave sai da arena";
}

TEST(WorldTest, GameOverFreezesTheSimulation)
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    for (int life = ast::World::kInitialLives; life > 0; --life)
    {
        advance(world, ast::World::kSpawnInvulnerability + kStep);
        world.addAsteroid(world.shipPosition(), {}, ast::AsteroidSize::Small);
        advanceUntilLifeLost(world, life);
    }

    ASSERT_EQ(world.outcome(), ast::World::Outcome::GameOver);

    const uint32_t rocks = world.asteroidCount();
    const uint32_t wave = world.wave();

    EXPECT_FALSE(world.fire()) << "sem nave, sem tiro";

    world.setThrust(true);
    advance(world, 2.0);

    // A arena congela como estava: nada de ondas novas nascendo sozinhas atras
    // da tela de game over.
    EXPECT_EQ(world.asteroidCount(), rocks);
    EXPECT_EQ(world.wave(), wave);
    EXPECT_EQ(world.outcome(), ast::World::Outcome::GameOver);
}

TEST(WorldTest, CrossingTheScoreThresholdGrantsAnExtraLife)
{
    ast::World world{ ast::WorldConfig{ .spawnAsteroids = false } };

    ASSERT_EQ(world.lives(), ast::World::kInitialLives);

    // Destroi rochas pequenas (kScoreSmall cada) ate cruzar o limiar do bonus.
    // A rocha nasce NA MIRA, nao em cima da nave — senao a nave morreria assim
    // que a protecao inicial acabasse, e o teste mediria outra coisa.
    const int needed = ast::World::kExtraLifeScore / ast::World::kScoreSmall;
    for (int i = 0; i < needed; ++i)
    {
        world.addAsteroid({ world.shipPosition().x, world.shipPosition().y - 120.0f }, {}, ast::AsteroidSize::Small);
        ASSERT_TRUE(world.fire()) << "tiro " << i;
        advanceUntilShotGone(world);
        advance(world, ast::World::kFireCooldown);
    }

    EXPECT_EQ(world.score(), ast::World::kExtraLifeScore);
    EXPECT_EQ(world.lives(), ast::World::kInitialLives + 1) << "o bonus ship do arcade";
}
