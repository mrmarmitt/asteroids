#pragma once

#include <cstdint>

// O dominio do Asteroids (task 02): a simulacao da nave — rotacao, thrust com
// inercia, wrap-around da arena e tiros com cooldown — em C++ puro, sem
// nenhuma dependencia de plataforma ou da cengine. A cena da plataforma
// traduz input em comandos, chama update(dt) com o passo fixo do engine e
// desenha lendo as consultas; os testes (CMake, sem The-Forge) dirigem a
// simulacao exatamente do mesmo jeito.
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

namespace ast {

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

struct WorldConfig
{
    /// Tiros herdam a velocidade da nave (comportamento do arcade). Testes de
    /// alcance/tempo de vida do tiro desligam para isolar a variavel.
    bool shotsInheritShipVelocity = true;
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

    /// Versor da proa — a cena desenha a nave com ele, os testes conferem a
    /// direcao do thrust e do tiro sem redescobrir a convencao de angulo.
    [[nodiscard]] Vec2 shipHeading() const;

    [[nodiscard]] uint32_t shotCount() const { return m_shotCount; }
    [[nodiscard]] Vec2     shotPosition(uint32_t index) const;

private:
    void updateShip(double dt);
    void updateShots(double dt);
    void removeShot(uint32_t index);

    /// Traz um ponto de volta para dentro da arena (toro: sai a direita, entra
    /// a esquerda). A regra vale para a nave e para os tiros.
    static Vec2 wrap(Vec2 point);

    WorldConfig m_config;

    Vec2   m_shipPos = { kArenaW * 0.5f, kArenaH * 0.5f };
    Vec2   m_shipVel = {};
    float  m_shipAngle = 0.0f; // nasce apontando para cima
    float  m_rotationAxis = 0.0f;
    bool   m_thrusting = false;
    double m_fireCooldown = 0.0;

    uint32_t m_shotCount = 0;
    Vec2     m_shotPos[kMaxShots] = {};
    Vec2     m_shotVel[kMaxShots] = {};
    double   m_shotLife[kMaxShots] = {};
};

} // namespace ast
