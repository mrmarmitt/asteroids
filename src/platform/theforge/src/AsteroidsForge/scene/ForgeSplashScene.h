#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;

// Splash (estado "initial"): titulo + "pressione ENTER" piscando.
class ForgeSplashScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter> m_gameRouter;
    double                      m_elapsed = 0.0;

public:
    explicit ForgeSplashScene(std::shared_ptr<GameRouter> gameRouter);

    void onEnter() override {}
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
