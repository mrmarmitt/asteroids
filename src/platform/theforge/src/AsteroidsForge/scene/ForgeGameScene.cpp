#include "ForgeGameScene.h"

#include <algorithm>
#include <utility>

#include "asteroids/game/GameRouter.h"

#include "ForgeUi.h"

namespace {
constexpr float kSpeed = 0.4f; // fracao da tela por segundo
} // namespace

ForgeGameScene::ForgeGameScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

void ForgeGameScene::update(const cengine::core::Seconds dt)
{
    // Movimento continuo por teclas SEGURADAS (heldAxis do common) dentro do
    // update de dt fixo — o mesmo caminho que o World da task 02 vai usar.
    const float step = kSpeed * static_cast<float>(dt.count());
    m_x = std::clamp(m_x + forgeui::heldAxis(Key::Left, Key::Right) * step, 0.05f, 0.95f);
    m_y = std::clamp(m_y + forgeui::heldAxis(Key::Up, Key::Down) * step, 0.10f, 0.90f);
}

void ForgeGameScene::draw()
{
    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    forgeui::drawText("^", m_x * w, m_y * h, 36.0f, forgeui::color::kSuccess);

    forgeui::drawText("CASCO (task 01) — World entra na task 02", 24.0f, 24.0f, 18.0f, forgeui::color::kFaint);
    forgeui::drawHints("SETAS mover   ESC voltar ao menu");
}

void ForgeGameScene::input()
{
    if (forgeui::readKey().key == Key::Escape)
    {
        m_gameRouter->menu();
    }
}
