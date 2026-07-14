#include "ForgeGameScene.h"

#include <cmath>
#include <cstdio>
#include <utility>

#include "asteroids/game/GameRouter.h"

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

ForgeGameScene::ForgeGameScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

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
}

void ForgeGameScene::draw()
{
    const ast::Vec2 shipPos = m_world.shipPosition();
    const float     angle = m_world.shipAngle();

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

    for (uint32_t i = 0; i < m_world.shotCount(); ++i)
    {
        drawDot(toScreen(m_world.shotPosition(i)), 14.0f, forgeui::color::kValue);
    }

    const ast::Vec2 velocity = m_world.shipVelocity();
    const float     speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

    char hud[64] = {};
    std::snprintf(hud, sizeof(hud), "ANG %3.0f   VEL %3.0f", angle * 57.2957795f, speed);
    forgeui::drawText(hud, 24.0f, 24.0f, 18.0f, forgeui::color::kDim);

    forgeui::drawHints("SETAS <- -> girar   SETA CIMA acelerar   ESPACO atirar   ESC voltar ao menu");
}
