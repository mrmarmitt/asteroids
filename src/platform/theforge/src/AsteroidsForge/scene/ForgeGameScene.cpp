#include "ForgeGameScene.h"

#include <cmath>
#include <cstdio>
#include <utility>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/PlaySession.h"

#include "ForgeUi.h"

namespace {

// A nave em coordenadas locais (proa em -Y, como o angulo 0 do World), em
// unidades da arena. Tres vertices desenhados como pontos: e o bastante para
// ler a rotacao na tela sem sprite nenhum.
constexpr ast::Vec2 kShipShape[] = {
    { 0.0f, -11.0f },  // proa
    { -7.0f, 8.0f },   // asa esquerda
    { 7.0f, 8.0f },    // asa direita
};
constexpr ast::Vec2 kThrustFlame = { 0.0f, 13.0f }; // atras da nave

constexpr float kTwoPi = 6.28318530718f;

// O World mantem os CENTROS dentro da arena, mas a borda de uma rocha pode
// passar dela — e essa borda tem de aparecer do outro lado, como o corpo.
ast::Vec2 wrapToArena(ast::Vec2 point)
{
    point.x = std::fmod(point.x + ast::World::kArenaW, ast::World::kArenaW);
    point.y = std::fmod(point.y + ast::World::kArenaH, ast::World::kArenaH);
    return point;
}

// Roda um ponto local pelo angulo da nave (mesma convencao do World: 0 = -Y,
// crescendo no sentido horario na tela).
ast::Vec2 rotate(const ast::Vec2 point, const float angle)
{
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    return { point.x * c - point.y * s, point.x * s + point.y * c };
}

void drawDot(const ast::Vec2 screen, const float size, const uint32_t color)
{
    // O glifo tem largura propria; centraliza no ponto para o triangulo nao
    // sair torto.
    forgeui::drawText("o", screen.x - size * 0.25f, screen.y - size * 0.5f, size, color);
}

} // namespace

ForgeGameScene::ForgeGameScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session)
    : m_gameRouter(std::move(gameRouter)), m_session(std::move(session))
{
}

ast::Vec2 ForgeGameScene::toScreen(const ast::Vec2 point) const
{
    // A arena e um toro que casa com as bordas da janela: escala direta, para
    // que o wrap-around do World aconteca exatamente onde a tela acaba.
    return { point.x * forgeui::screenWidth() / ast::World::kArenaW,
             point.y * forgeui::screenHeight() / ast::World::kArenaH };
}

void ForgeGameScene::input()
{
    // Teclas SEGURADAS viram comandos continuos do World; a fila de edges fica
    // so para as transicoes de estado.
    m_world.setRotationAxis(forgeui::heldAxis(Key::Left, Key::Right));
    m_world.setThrust(forgeui::isHeld(Key::Up));

    if (forgeui::isHeld(Key::Space))
    {
        m_world.fire(); // o cooldown do World e quem decide se sai tiro
    }

    if (forgeui::readKey().key == Key::Escape)
    {
        m_gameRouter->menu();
    }
}

void ForgeGameScene::update(const cengine::core::Seconds dt)
{
    m_world.update(dt.count());
    m_elapsed += dt.count();

    // Acabaram as vidas: o World so CONSTATA; quem decide o que a derrota
    // significa e o fluxo. Publica o resultado (o World morre com a cena) e
    // roteia.
    if (m_world.outcome() == ast::World::Outcome::GameOver)
    {
        m_session->setResult(m_world.score(), m_world.wave());
        m_gameRouter->gameOver();
    }
}

void ForgeGameScene::draw()
{
    // Rochas: um anel de pontos com o raio que o World diz — ainda sem sprite
    // nenhum (a escolha do renderizador segue adiada; ver task 02).
    for (uint32_t i = 0; i < m_world.asteroidCount(); ++i)
    {
        const ast::Vec2 center = m_world.asteroidPosition(i);
        const float     radius = ast::World::asteroidRadius(m_world.asteroidSize(i));
        const uint32_t  points = radius > 30.0f ? 12u : (radius > 15.0f ? 8u : 6u);

        for (uint32_t p = 0; p < points; ++p)
        {
            const float angle = kTwoPi * static_cast<float>(p) / static_cast<float>(points);
            const ast::Vec2 rim = rotate({ 0.0f, -radius }, angle);

            // O anel pode atravessar a borda: passa pelo wrap do World para o
            // pedaco que sai por um lado aparecer no outro, como a rocha.
            drawDot(toScreen(wrapToArena({ center.x + rim.x, center.y + rim.y })), 12.0f, forgeui::color::kDim);
        }
    }

    const ast::Vec2 shipPos = m_world.shipPosition();
    const float     angle = m_world.shipAngle();

    // Protegida logo apos (re)nascer: pisca, como no arcade. Sem vidas, some.
    const bool blinkOff = m_world.shipInvulnerable() && std::fmod(m_elapsed, 0.24) < 0.12;
    if (m_world.shipAlive() && !blinkOff)
    {
        for (const ast::Vec2 vertex : kShipShape)
        {
            const ast::Vec2 local = rotate(vertex, angle);
            drawDot(toScreen({ shipPos.x + local.x, shipPos.y + local.y }), 20.0f, forgeui::color::kSuccess);
        }

        if (m_world.thrusting())
        {
            const ast::Vec2 local = rotate(kThrustFlame, angle);
            drawDot(toScreen({ shipPos.x + local.x, shipPos.y + local.y }), 16.0f, forgeui::color::kAccent);
        }
    }

    for (uint32_t i = 0; i < m_world.shotCount(); ++i)
    {
        drawDot(toScreen(m_world.shotPosition(i)), 14.0f, forgeui::color::kValue);
    }

    // HUD: pontos a esquerda, vidas em naves (como no arcade), onda a direita.
    char score[32] = {};
    std::snprintf(score, sizeof(score), "%06d", m_world.score());
    forgeui::drawText(score, 24.0f, 24.0f, 24.0f, forgeui::color::kValue);

    std::string ships;
    for (int i = 0; i < m_world.lives(); ++i)
    {
        ships += "^ ";
    }
    forgeui::drawText(ships, 24.0f, 56.0f, 22.0f, forgeui::color::kSuccess);

    char wave[32] = {};
    std::snprintf(wave, sizeof(wave), "ONDA %u", m_world.wave());
    forgeui::drawText(wave, forgeui::screenWidth() - forgeui::textWidth(wave, 18.0f) - 24.0f, 24.0f, 18.0f,
                      forgeui::color::kDim);

    forgeui::drawHints("SETAS <- -> girar   SETA CIMA acelerar   ESPACO atirar   ESC voltar ao menu");
}
