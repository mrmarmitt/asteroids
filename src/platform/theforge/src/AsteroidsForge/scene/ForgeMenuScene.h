#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;

// Menu (estado "menu"): JOGAR / SAIR com cursor de texto.
class ForgeMenuScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter> m_gameRouter;
    int                         m_selected = 0;

public:
    explicit ForgeMenuScene(std::shared_ptr<GameRouter> gameRouter);

    void onEnter() override {}
    void update(cengine::core::Seconds) override {}
    void draw() override;
    void input() override;
    void onExit() override {}
};
