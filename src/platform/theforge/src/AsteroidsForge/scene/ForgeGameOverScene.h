#pragma once

#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;
class PlaySession;

namespace ast {
class RecordService;
}

// O estado "gameover": mostra o resultado da partida que acabou (lido do
// PlaySession — a cena do jogo, e o World dela, ja morreram) e, se a pontuacao
// entrou no top-N, pede o nome ali mesmo antes de gravar.
//
// Duas saidas conforme o caso:
//   recorde   -> digita o nome, ENTER grava e mostra a tabela
//   sem recorde -> ENTER joga de novo, ESC volta ao menu
class ForgeGameOverScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>         m_gameRouter;
    std::shared_ptr<PlaySession>        m_session;
    std::shared_ptr<ast::RecordService> m_records;

    bool        m_isNewRecord = false; // decidido no onEnter, quando o placar ja existe
    std::string m_name;

    double m_elapsed = 0.0; // so para piscar (dica e cursor)

    void saveRecord();

public:
    ForgeGameOverScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session,
                       std::shared_ptr<ast::RecordService> records);

    void onEnter() override;
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
