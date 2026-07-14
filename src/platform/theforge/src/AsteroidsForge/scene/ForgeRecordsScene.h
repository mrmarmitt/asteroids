#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;

namespace ast {
class RecordService;
}

// O estado "records": a tabela do top-N, lida do RecordService. Alcancavel pelo
// menu e logo depois de gravar um recorde novo.
class ForgeRecordsScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>         m_gameRouter;
    std::shared_ptr<ast::RecordService> m_records;

public:
    ForgeRecordsScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<ast::RecordService> records);

    void onEnter() override {}
    void update(cengine::core::Seconds) override {}
    void draw() override;
    void input() override;
    void onExit() override {}
};
