#pragma once

#include <cstdint>

// O dominio do Asteroids: a simulacao da partida — nave (rotacao, thrust com
// inercia, wrap-around, tiros; task 02) e as rochas (ondas, fragmentacao e
// colisao; task 03) — em C++ puro, sem nenhuma dependencia de plataforma ou da
// cengine. A cena da plataforma traduz input em comandos, chama update(dt) com
// o passo fixo do engine e desenha lendo as consultas; os testes (CMake, sem
// The-Forge) dirigem a simulacao exatamente do mesmo jeito.
//
// Coordenadas: arena virtual de 800x600. Origem no canto superior esquerdo,
// Y crescendo PARA BAIXO (mesma convencao do World do spaceinvaders e das
// telas); quem projeta para a resolucao real e a cena.
//
// Angulo: radianos, 0 = nave apontando para CIMA (-Y), crescendo no sentido
// horario na tela (o sentido que a seta direita produz). Portanto o versor da
// proa e (sin a, -cos a).
//
// Fisica (o "sentir" do arcade, nao Newton exato): a rotacao e instantanea em
// velocidade; o thrust acelera na direcao da proa; um atrito exponencial
// suave freia a nave quando o thrust solta — sem ele a nave fica ingovernavel
// e o jogo vira um teste de paciencia. A velocidade satura em kMaxSpeed.
//
// Colisao: a DETECCAO (circulo x circulo) e da engine —
// `cengine::collision2d::intersects` — mas o TORO e nosso. Arena que da a volta
// e politica do jogo: calculamos o menor delta na nossa topologia e perguntamos
// a engine com a posicao ja corrigida. A engine nao tem (nem deve ter) opiniao
// sobre o formato do mundo. Ver a Emenda 1 da ADR 0002 da cengine.

#include <cengine/collision2d/Shapes.hpp>

namespace ast {

/// O ponto 2D do jogo e o da engine: um alias, nao uma copia — assim uma posicao
/// do World entra direto num `Circle` da colisao, sem conversao nem espelho.
using Vec2 = cengine::collision2d::Vec2;

/// O tamanho de uma rocha define raio, velocidade e no que ela se parte.
enum class AsteroidSize : uint8_t
{
    Large,
    Medium,
    Small,
};

struct WorldConfig
{
    uint32_t seed = 1; // RNG das rochas (deterministico p/ testes)

    /// Tiros herdam a velocidade da nave (comportamento do arcade). Testes de
    /// alcance/tempo de vida do tiro desligam para isolar a variavel.
    bool shotsInheritShipVelocity = true;

    /// Testes que so querem exercitar a nave nascem com a arena vazia.
    bool spawnAsteroids = true;
};

class World
{
public:
    static constexpr float kArenaW = 800.0f;
    static constexpr float kArenaH = 600.0f;

    static constexpr uint32_t kMaxShots = 4; // regra do arcade: poucos tiros em voo

    // --- constantes de "sentir" (unidades da arena por segundo) ---
    static constexpr float  kRotationSpeed = 3.6f;  // rad/s (uma volta em ~1.75s)
    static constexpr float  kThrustAccel = 320.0f;  // aceleracao da proa
    static constexpr float  kMaxSpeed = 340.0f;     // saturacao da velocidade
    static constexpr float  kDrag = 0.45f;          // atrito exponencial por segundo
    static constexpr float  kShotSpeed = 480.0f;
    static constexpr double kShotLife = 1.15;       // segundos ate o tiro sumir
    static constexpr double kFireCooldown = 0.22;   // segundos entre disparos

    static constexpr float kShipRadius = 9.0f;
    static constexpr float kShotRadius = 2.0f;

    // --- rochas ---
    static constexpr uint32_t kMaxAsteroids = 64;      // teto: onda cheia toda fragmentada
    static constexpr uint32_t kAsteroidsFirstWave = 4; // +1 rocha por onda
    static constexpr uint32_t kMaxAsteroidsPerWave = 9;

    /// Raio livre ao redor da nave onde uma rocha nova nunca nasce (senao a
    /// onda comeca com uma morte injusta).
    static constexpr float kSpawnSafeRadius = 140.0f;

    /// Depois de ser atingida, a nave volta ao centro e fica intocavel por um
    /// tempo — sem isso ela morreria de novo na mesma rocha, no mesmo quadro.
    static constexpr double kSpawnInvulnerability = 2.0;

    [[nodiscard]] static float asteroidRadius(AsteroidSize size);

    explicit World(WorldConfig config = {});

    // --- comandos (fase input da cena) ---

    /// Rotacao neste quadro: -1 anti-horario (esquerda), 0 parado, +1 horario.
    /// Valores fora de [-1, 1] sao saturados.
    void setRotationAxis(float axis);

    /// Motor ligado/desligado neste quadro (tecla SEGURADA na cena).
    void setThrust(bool thrusting);

    /// Dispara se ha vaga (kMaxShots) e o cooldown zerou. Retorna se saiu tiro.
    bool fire();

    // --- simulacao (fase update, dt fixo do engine) ---
    void update(double dt);

    // --- consultas (fase draw e testes) ---
    [[nodiscard]] Vec2  shipPosition() const { return m_shipPos; }
    [[nodiscard]] Vec2  shipVelocity() const { return m_shipVel; }
    [[nodiscard]] float shipAngle() const { return m_shipAngle; }
    [[nodiscard]] bool  thrusting() const { return m_thrusting; }

    /// Nave intocavel logo apos (re)nascer — a cena pisca a nave enquanto dura.
    [[nodiscard]] bool shipInvulnerable() const { return m_invulnerability > 0.0; }

    /// Quantas vezes a nave foi atingida. Vidas/gameOver sao da task 04; por
    /// ora isto e so o contador que prova que a colisao nave x rocha acontece.
    [[nodiscard]] uint32_t shipHits() const { return m_shipHits; }

    /// Versor da proa — a cena desenha a nave com ele, os testes conferem a
    /// direcao do thrust e do tiro sem redescobrir a convencao de angulo.
    [[nodiscard]] Vec2 shipHeading() const;

    [[nodiscard]] uint32_t shotCount() const { return m_shotCount; }
    [[nodiscard]] Vec2     shotPosition(uint32_t index) const;

    [[nodiscard]] uint32_t     asteroidCount() const { return m_asteroidCount; }
    [[nodiscard]] Vec2         asteroidPosition(uint32_t index) const;
    [[nodiscard]] AsteroidSize asteroidSize(uint32_t index) const;

    /// Onda atual (1-based). A arena limpa faz a proxima nascer, com uma
    /// rocha a mais.
    [[nodiscard]] uint32_t wave() const { return m_wave; }

    // --- a topologia da arena: o pedaco que a engine NAO sabe ---

    /// Menor vetor de `from` ate `to` NA ARENA-TORO: atravessar a borda pode
    /// ser o caminho curto. E o que faz a colisao funcionar em quem deu a volta
    /// — e e POLITICA DO JOGO (a cengine nao opina sobre o formato do mundo).
    [[nodiscard]] static Vec2 toroidalDelta(Vec2 from, Vec2 to);

    /// Circulos se tocam na arena-toro? Corrige a posicao de `b` pelo menor
    /// delta e delega a DETECCAO para `cengine::collision2d::intersects`.
    [[nodiscard]] static bool circlesOverlap(Vec2 a, float radiusA, Vec2 b, float radiusB);

    // --- ganchos de teste (mesmo padrao do killInvader do spaceinvaders):
    //     montam uma arena controlada, sem depender do sorteio da onda ---

    /// Poe uma rocha exatamente onde o teste quer.
    void addAsteroid(Vec2 position, Vec2 velocity, AsteroidSize size);

    /// Esvazia a arena (com as ondas ligadas, o proximo update faz a seguinte
    /// nascer — que e justamente a regra a testar).
    void clearAsteroids() { m_asteroidCount = 0; }

private:
    void updateShip(double dt);
    void updateShots(double dt);
    void updateAsteroids(double dt);

    /// Tiro x rocha: parte a rocha atingida e consome o tiro.
    void collideShotsWithAsteroids();
    /// Nave x rocha: parte a rocha e devolve a nave ao centro, intocavel.
    void collideShipWithAsteroids();

    void spawnWave();

    /// Parte a rocha em dois fragmentos menores (ou some, se ja era pequena).
    void splitAsteroid(uint32_t index);
    void removeAsteroid(uint32_t index);
    void removeShot(uint32_t index);
    void respawnShip();

    /// Traz um ponto de volta para dentro da arena (toro: sai a direita, entra
    /// a esquerda). A regra vale para a nave, para os tiros e para as rochas.
    static Vec2 wrap(Vec2 point);

    /// LCG deterministico — a mesma seed da a mesma partida, e os testes
    /// conseguem prever a onda.
    float rand01();
    float randRange(float min, float max);

    WorldConfig m_config;
    uint32_t    m_rng = 1;

    Vec2   m_shipPos = { kArenaW * 0.5f, kArenaH * 0.5f };
    Vec2   m_shipVel = {};
    float  m_shipAngle = 0.0f; // nasce apontando para cima
    float  m_rotationAxis = 0.0f;
    bool   m_thrusting = false;
    double m_fireCooldown = 0.0;
    double m_invulnerability = kSpawnInvulnerability;
    uint32_t m_shipHits = 0;

    uint32_t m_shotCount = 0;
    Vec2     m_shotPos[kMaxShots] = {};
    Vec2     m_shotVel[kMaxShots] = {};
    double   m_shotLife[kMaxShots] = {};

    uint32_t     m_asteroidCount = 0;
    Vec2         m_asteroidPos[kMaxAsteroids] = {};
    Vec2         m_asteroidVel[kMaxAsteroids] = {};
    AsteroidSize m_asteroidSize[kMaxAsteroids] = {};

    uint32_t m_wave = 1;
};

} // namespace ast
