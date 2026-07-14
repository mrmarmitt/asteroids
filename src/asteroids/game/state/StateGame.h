#pragma once

#include <memory>
#include <string>

#include <cengine/routing/StateCodes.hpp>

#include "asteroids/game/GameRouter.h"
#include "StateGameFlow.h"

// Fluxo do Asteroids (task 05):
//
//   initial (splash) -> menu -> game -> gameOver -> records -> menu
//                        |  \             |   \--> menu           ^
//                        |   \            \--> game (de novo)     |
//                        |    \--> menu (ESC abandona o jogo)     |
//                        \--> records ----------------------------/
//                        \--> exit
//
// Codigos de estado = chaves das factories de cena (ForgeSceneFactory).
// "exit" casa com cengine::routing::kExitStateCode — shouldExit() vira true
// e o start() da cengine encerra com cleanup().

class InitialSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "initial"; }
    [[nodiscard]] std::string getName() const override { return "Splash"; }

    void menu(const GameRouter& game) const override; // unica transicao valida
    void game(const GameRouter&) const override {}
    void gameOver(const GameRouter&) const override {}
    void records(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

class MenuSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "menu"; }
    [[nodiscard]] std::string getName() const override { return "Menu"; }

    void menu(const GameRouter&) const override {}
    void game(const GameRouter& game) const override;
    void gameOver(const GameRouter&) const override {}
    void records(const GameRouter& game) const override;
    void exit(const GameRouter& game) const override;
};

class GameSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "game"; }
    [[nodiscard]] std::string getName() const override { return "Game"; }

    void menu(const GameRouter& game) const override; // abandono via ESC
    void game(const GameRouter&) const override {}
    void gameOver(const GameRouter& game) const override; // acabaram as vidas
    void records(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

class GameOverSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "gameover"; }
    [[nodiscard]] std::string getName() const override { return "Game Over"; }

    void menu(const GameRouter& game) const override;
    void game(const GameRouter& game) const override;    // jogar de novo (partida zerada)
    void gameOver(const GameRouter&) const override {}
    void records(const GameRouter& game) const override; // depois de gravar o recorde
    void exit(const GameRouter&) const override {}
};

class RecordsSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "records"; }
    [[nodiscard]] std::string getName() const override { return "Recordes"; }

    void menu(const GameRouter& game) const override;
    void game(const GameRouter&) const override {}
    void gameOver(const GameRouter&) const override {}
    void records(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

class ExitSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return std::string{cengine::routing::kExitStateCode}; }
    [[nodiscard]] std::string getName() const override { return "Exit"; }

    void menu(const GameRouter&) const override {}
    void game(const GameRouter&) const override {}
    void gameOver(const GameRouter&) const override {}
    void records(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

inline void InitialSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }

inline void MenuSG::game(const GameRouter& game) const { game.setNextState(std::make_unique<GameSG>()); }
inline void MenuSG::records(const GameRouter& game) const { game.setNextState(std::make_unique<RecordsSG>()); }
inline void MenuSG::exit(const GameRouter& game) const { game.setNextState(std::make_unique<ExitSG>()); }

inline void GameSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
inline void GameSG::gameOver(const GameRouter& game) const { game.setNextState(std::make_unique<GameOverSG>()); }

inline void GameOverSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
inline void GameOverSG::game(const GameRouter& game) const { game.setNextState(std::make_unique<GameSG>()); }
inline void GameOverSG::records(const GameRouter& game) const { game.setNextState(std::make_unique<RecordsSG>()); }

inline void RecordsSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
