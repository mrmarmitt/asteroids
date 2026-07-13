#include "ForgeSplashScene.h"

#include <cmath>
#include <utility>

#include "asteroids/game/GameRouter.h"

#include "ForgeUi.h"

ForgeSplashScene::ForgeSplashScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

void ForgeSplashScene::update(const cengine::core::Seconds dt) { m_elapsed += dt.count(); }

void ForgeSplashScene::draw()
{
    const float h = forgeui::screenHeight();

    forgeui::drawTextCentered("A S T E R O I D S", h * 0.30f, 64.0f, forgeui::color::kTitle);
    forgeui::drawTextCentered("cengine 0.6.0 + platform-theforge-common 0.1.0", h * 0.42f, 20.0f,
                              forgeui::color::kDim);

    // piscar dirigido pelo tempo de simulacao (update com dt fixo)
    if (std::fmod(m_elapsed, 1.2) < 0.8)
    {
        forgeui::drawTextCentered("Pressione ENTER para comecar", h * 0.66f, 24.0f, forgeui::color::kValue);
    }

    forgeui::drawHints("ENTER comecar");
}

void ForgeSplashScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Enter:
    case Key::Escape:
        m_gameRouter->menu(); // sair so existe a partir do menu
        break;
    default:
        break;
    }
}
