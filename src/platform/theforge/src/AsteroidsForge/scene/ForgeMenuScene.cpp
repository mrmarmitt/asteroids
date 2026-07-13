#include "ForgeMenuScene.h"

#include <utility>

#include "asteroids/game/GameRouter.h"

#include "ForgeUi.h"

namespace {
constexpr const char* kOptions[] = { "JOGAR", "SAIR" };
constexpr int         kOptionCount = 2;
} // namespace

ForgeMenuScene::ForgeMenuScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

void ForgeMenuScene::draw()
{
    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    const float top = h * 0.46f;
    const float rowH = 56.0f;

    forgeui::drawTextCentered("A S T E R O I D S", h * 0.20f, 44.0f, forgeui::color::kTitle);

    for (int i = 0; i < kOptionCount; ++i)
    {
        const bool     selected = i == m_selected;
        const uint32_t color = selected ? forgeui::color::kAccent : forgeui::color::kText;
        forgeui::drawTextCentered(kOptions[i], top + i * rowH, 30.0f, color);
        if (selected)
        {
            const float textW = forgeui::textWidth(kOptions[i], 30.0f);
            forgeui::drawText(">", (w - textW) * 0.5f - 48.0f, top + i * rowH, 30.0f, forgeui::color::kAccent);
        }
    }

    forgeui::drawHints("SETAS navegar   ENTER confirmar   ESC sair");
}

void ForgeMenuScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Up:
        m_selected = (m_selected + kOptionCount - 1) % kOptionCount;
        break;
    case Key::Down:
        m_selected = (m_selected + 1) % kOptionCount;
        break;
    case Key::Enter:
        if (m_selected == 0)
            m_gameRouter->game();
        else
            m_gameRouter->exit();
        break;
    case Key::Escape:
        m_gameRouter->exit();
        break;
    default:
        break;
    }
}
