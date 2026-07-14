#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

#include "asteroids/game/World.h"

class GameRouter;

// O estado "game" (task 02): a cena e so a casca — traduz o input segurado em
// comandos do World, chama update(dt) com o passo fixo do engine e desenha
// lendo as consultas. Nenhuma regra de jogo mora aqui.
//
// Desenho ainda por TEXTO (o batcher de sprites segue desligado): a nave e um
// triangulo de pontos rotacionado pelo angulo do World — o suficiente para
// validar rotacao, inercia e wrap-around de olho, sem decidir ainda como o
// asteroids sera renderizado de verdade (ver task 03).
class ForgeGameScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter> m_gameRouter;
    ast::World                  m_world;

    /// Relogio so de apresentacao (pisca-pisca da nave protegida) — nao entra
    /// na simulacao.
    double m_elapsed = 0.0;

    /// Projeta um ponto da arena do World (800x600) para pixels da tela.
    [[nodiscard]] ast::Vec2 toScreen(ast::Vec2 point) const;

public:
    explicit ForgeGameScene(std::shared_ptr<GameRouter> gameRouter);

    void onEnter() override {}
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
