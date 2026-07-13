#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;

// Placeholder do gameplay (estado "game") — degrau do casco (task 01): uma
// "nave" de texto move com as setas SEGURADAS, validando o caminho
// isHeld/heldAxis do platform-theforge-common. O World de verdade (rotacao,
// inercia, tiros) entra na task 02.
class ForgeGameScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter> m_gameRouter;
    float                       m_x = 0.5f; // posicao normalizada (0..1)
    float                       m_y = 0.5f;

public:
    explicit ForgeGameScene(std::shared_ptr<GameRouter> gameRouter);

    void onEnter() override {}
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
