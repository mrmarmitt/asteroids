#pragma once

// Fachada de navegacao de dominio do Asteroids sobre o roteador da cengine.
// Diferente dos irmaos (8puzzle/spaceinvaders, que carregavam uma copia da
// mecanica), aqui so existe o VOCABULARIO: a mecanica (guardar o router,
// castar o estado atual, delegar o agendamento) vem do FlowRouter da
// cengine 0.6.0 (task 19 de la — este jogo e o primeiro consumidor).

#include <cengine/routing/FlowRouter.hpp>

#include "state/StateGameFlow.h"

class GameRouter final: public cengine::routing::FlowRouter<StateGameFlow>
{
public:
    using FlowRouter::FlowRouter;

    // As transicoes despacham sobre o estado ATUAL do router.
    void menu() const { current().menu(*this); }
    void game() const { current().game(*this); }
    void exit() const { current().exit(*this); }
};
