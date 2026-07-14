#pragma once

#include <cengine/routing/IState.hpp>

class GameRouter;

// Estado da maquina de fluxo do jogo (mesmo desenho dos irmaos): estados
// sem dados, cada transicao valida despacha a proxima cena via GameRouter;
// transicoes invalidas sao no-ops explicitos.
//
// O vocabulario cresce por degrau: o bootstrap tinha menu/game/exit; a task 04
// (pontuacao/vidas) trouxe o gameOver.
class StateGameFlow: public cengine::routing::IState
{
public:
    StateGameFlow() = default;
    ~StateGameFlow() override = default;

    [[nodiscard]] std::string getCode() const override = 0;
    [[nodiscard]] std::string getName() const override = 0;

    virtual void menu(const GameRouter& game) const = 0;
    virtual void game(const GameRouter& game) const = 0;
    virtual void gameOver(const GameRouter& game) const = 0;
    virtual void exit(const GameRouter& game) const = 0;
};
