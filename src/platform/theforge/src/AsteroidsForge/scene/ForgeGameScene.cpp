#include "ForgeGameScene.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <utility>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/PlaySession.h"

#include "ForgeLineUi.h"
#include "ForgeUi.h"

namespace {

// A nave em coordenadas locais (proa em -Y, como o angulo 0 do World), em
// unidades da arena — o mesmo triangulo do arcade, com a popa reentrante.
constexpr ast::Vec2 kShipShape[] = {
    { 0.0f, -12.0f },  // proa
    { 8.0f, 10.0f },   // asa direita
    { 0.0f, 6.0f },    // popa (o "V" que da a silhueta classica)
    { -8.0f, 10.0f },  // asa esquerda
};

// A chama do motor: um triangulo atras da nave, so quando o thrust esta ligado.
constexpr ast::Vec2 kFlameShape[] = {
    { -4.0f, 10.0f },
    { 0.0f, 18.0f },
    { 4.0f, 10.0f },
};

constexpr float kTwoPi = 6.28318530718f;

// Quantos vertices tem cada tamanho de rocha (o arcade usava poligonos
// irregulares de ~10 lados; menor = menos lados).
constexpr uint32_t kRockVertices[] = { 12, 10, 8 };
constexpr uint32_t kMaxRockVertices = 12;

// Roda um ponto local pelo angulo (mesma convencao do World: 0 = -Y, crescendo
// no sentido horario na tela).
ast::Vec2 rotate(const ast::Vec2 point, const float angle)
{
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    return { point.x * c - point.y * s, point.x * s + point.y * c };
}

// Ruido deterministico por (rocha, vertice): a MESMA rocha tem sempre a mesma
// silhueta irregular, quadro apos quadro — sem guardar nada, e sem tremer.
float jitter(const uint32_t seed, const uint32_t vertex)
{
    uint32_t h = seed * 374761393u + vertex * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<float>((h ^ (h >> 16)) & 0xFFFFu) / 65535.0f; // 0..1
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

// Desenha um poligono do JOGO (pontos locais, em unidades da arena) girado,
// transladado e projetado — em UMA polyline fechada.
//
// O corpo pode atravessar a borda da arena: como a arena e um toro, desenhamos
// o poligono ATE 4 vezes (deslocado por +-arena em X e Y) e deixamos o que cai
// fora da tela ser recortado. E o preco de duas linhas por o wrap-around
// aparecer inteiro, em vez de um objeto cortado pela metade na borda.
void ForgeGameScene::drawWrapped(const ast::Vec2* shape, const uint32_t count, const ast::Vec2 center, const float angle,
                                 const uint32_t color) const
{
    const bool nearLeft = center.x < ast::World::kArenaW * 0.5f;
    const bool nearTop = center.y < ast::World::kArenaH * 0.5f;

    const float offsetX = nearLeft ? ast::World::kArenaW : -ast::World::kArenaW;
    const float offsetY = nearTop ? ast::World::kArenaH : -ast::World::kArenaH;

    const ast::Vec2 copies[] = {
        { center.x, center.y },
        { center.x + offsetX, center.y },
        { center.x, center.y + offsetY },
        { center.x + offsetX, center.y + offsetY },
    };

    for (const ast::Vec2 origin : copies)
    {
        forgeline::Point screen[kMaxRockVertices + 1] = {};
        for (uint32_t i = 0; i < count; ++i)
        {
            const ast::Vec2 local = rotate(shape[i], angle);
            const ast::Vec2 point = toScreen({ origin.x + local.x, origin.y + local.y });
            screen[i] = { point.x, point.y };
        }
        forgeline::drawPolyline(screen, count, true, color);
    }
}

void ForgeGameScene::draw()
{
    // --- rochas: poligonos irregulares, um por rocha ---
    for (uint32_t i = 0; i < m_world.asteroidCount(); ++i)
    {
        const ast::AsteroidSize size = m_world.asteroidSize(i);
        const float             radius = ast::World::asteroidRadius(size);
        const uint32_t          vertices = kRockVertices[static_cast<size_t>(size)];

        // A semente vem da POSICAO inicial arredondada: e estavel enquanto a
        // rocha existe e diferente entre rochas, sem o World precisar guardar
        // nada de desenho (a silhueta e assunto da cena, nao da simulacao).
        const ast::Vec2 center = m_world.asteroidPosition(i);
        const auto      seed = static_cast<uint32_t>(i * 2654435761u);

        ast::Vec2 shape[kMaxRockVertices] = {};
        for (uint32_t v = 0; v < vertices; ++v)
        {
            const float angle = kTwoPi * static_cast<float>(v) / static_cast<float>(vertices);
            const float bump = 0.72f + 0.28f * jitter(seed, v); // 72%..100% do raio
            shape[v] = { std::sin(angle) * radius * bump, -std::cos(angle) * radius * bump };
        }

        drawWrapped(shape, vertices, center, 0.0f, forgeui::color::kDim);
    }

    // --- tiros: risquinhos na direcao do voo ---
    for (uint32_t i = 0; i < m_world.shotCount(); ++i)
    {
        const ast::Vec2 shot = toScreen(m_world.shotPosition(i));
        forgeline::drawLine({ shot.x, shot.y }, { shot.x + 2.0f, shot.y + 2.0f }, forgeui::color::kValue);
    }

    // --- nave: o triangulo do arcade, piscando enquanto protegida ---
    const bool blinkOff = m_world.shipInvulnerable() && std::fmod(m_elapsed, 0.24) < 0.12;
    if (m_world.shipAlive() && !blinkOff)
    {
        const ast::Vec2 position = m_world.shipPosition();
        const float     angle = m_world.shipAngle();

        drawWrapped(kShipShape, 4, position, angle, forgeui::color::kSuccess);

        if (m_world.thrusting())
        {
            drawWrapped(kFlameShape, 3, position, angle, forgeui::color::kAccent);
        }
    }

    // --- HUD (texto por cima: o drawText da flush no lote de linhas) ---
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
