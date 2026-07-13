#pragma once

#include <memory>
#include <string>

#include <cengine/routing/StateCodes.hpp>

#include "asteroids/game/GameRouter.h"
#include "StateGameFlow.h"

// Fluxo do Asteroids (bootstrap — task 01):
//
//   initial (splash) -> menu -> game -> menu (ESC abandona)
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
    void exit(const GameRouter&) const override {}
};

class MenuSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "menu"; }
    [[nodiscard]] std::string getName() const override { return "Menu"; }

    void menu(const GameRouter&) const override {}
    void game(const GameRouter& game) const override;
    void exit(const GameRouter& game) const override;
};

class GameSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "game"; }
    [[nodiscard]] std::string getName() const override { return "Game"; }

    void menu(const GameRouter& game) const override; // abandono via ESC
    void game(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

class ExitSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return std::string{cengine::routing::kExitStateCode}; }
    [[nodiscard]] std::string getName() const override { return "Exit"; }

    void menu(const GameRouter&) const override {}
    void game(const GameRouter&) const override {}
    void exit(const GameRouter&) const override {}
};

inline void InitialSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }

inline void MenuSG::game(const GameRouter& game) const { game.setNextState(std::make_unique<GameSG>()); }
inline void MenuSG::exit(const GameRouter& game) const { game.setNextState(std::make_unique<ExitSG>()); }

inline void GameSG::menu(const GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
