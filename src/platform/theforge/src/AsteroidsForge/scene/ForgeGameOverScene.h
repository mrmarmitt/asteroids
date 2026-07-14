#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;
class PlaySession;

// O estado "gameover": mostra o resultado da partida que acabou (lido do
// PlaySession — a cena do jogo, e o World dela, ja morreram) e oferece as duas
// saidas: jogar de novo (partida zerada) ou voltar ao menu.
class ForgeGameOverScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>  m_gameRouter;
    std::shared_ptr<PlaySession> m_session;

    double m_elapsed = 0.0; // so para piscar a dica

public:
    ForgeGameOverScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session);

    void onEnter() override {}
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
