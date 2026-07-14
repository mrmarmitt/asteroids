#include "ForgeRecordsScene.h"

#include <cstdio>
#include <utility>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/RecordService.h"

#include "ForgeUi.h"

ForgeRecordsScene::ForgeRecordsScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<ast::RecordService> records)
    : m_gameRouter(std::move(gameRouter)), m_records(std::move(records))
{
}

void ForgeRecordsScene::draw()
{
    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    forgeui::drawTextCentered("R E C O R D E S", h * 0.14f, 40.0f, forgeui::color::kTitle);

    const auto& ranked = m_records->listByScore();

    if (ranked.empty())
    {
        forgeui::drawTextCentered("nenhuma partida registrada ainda", h * 0.45f, 22.0f, forgeui::color::kFaint);
        forgeui::drawHints("ESC voltar ao menu");
        return;
    }

    // Tabela em colunas de posicao fixa (a fonte nao e monoespacada, entao cada
    // coluna e desenhada no seu proprio x).
    const float left = w * 0.22f;
    const float top = h * 0.30f;
    const float rowH = 32.0f;

    for (size_t i = 0; i < ranked.size(); ++i)
    {
        const auto& record = ranked[i];
        const float y = top + static_cast<float>(i) * rowH;

        // O primeiro colocado em destaque — e o unico que o jogador quer bater.
        const uint32_t color = i == 0 ? forgeui::color::kAccent : forgeui::color::kText;

        char position[8] = {};
        std::snprintf(position, sizeof(position), "%zu.", i + 1);
        forgeui::drawText(position, left, y, 22.0f, forgeui::color::kDim);

        forgeui::drawText(record.name(), left + 48.0f, y, 22.0f, color);

        char score[16] = {};
        std::snprintf(score, sizeof(score), "%06d", record.score());
        forgeui::drawText(score, left + 200.0f, y, 22.0f, forgeui::color::kValue);

        char wave[16] = {};
        std::snprintf(wave, sizeof(wave), "onda %u", record.wave());
        forgeui::drawText(wave, left + 320.0f, y, 22.0f, forgeui::color::kDim);

        forgeui::drawText(record.playedAt(), left + 430.0f, y, 20.0f, forgeui::color::kFaint);
    }

    forgeui::drawHints("ESC voltar ao menu");
}

void ForgeRecordsScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Escape:
    case Key::Enter:
        m_gameRouter->menu();
        break;
    default:
        break;
    }
}
