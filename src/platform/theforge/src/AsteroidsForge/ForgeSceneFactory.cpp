#include "ForgeSceneFactory.h"

#include <cengine/routing/ISceneRepository.hpp>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/PlaySession.h"
#include "asteroids/game/service/RecordService.h"

#include "scene/ForgeExitScene.h"
#include "scene/ForgeGameOverScene.h"
#include "scene/ForgeGameScene.h"
#include "scene/ForgeMenuScene.h"
#include "scene/ForgeRecordsScene.h"
#include "scene/ForgeSplashScene.h"

// As factories rodam LAZY (no primeiro getScene de cada estado) e a cena morre
// ao sair do estado (o router a descarrega no commit) — capturas por VALOR.
// Dai a partida nova nascer zerada de graca.
//
// O que precisa ATRAVESSAR a troca de cena vive fora delas: o PlaySession (o
// resultado da ultima partida) e o RecordService (a tabela).
void ForgeSceneFactory::populateForgeScenes(cengine::routing::ISceneRepository&        sceneRepository,
                                            const std::shared_ptr<GameRouter>&         gameRouter,
                                            const std::shared_ptr<PlaySession>&        session,
                                            const std::shared_ptr<ast::RecordService>& records)
{
    sceneRepository.registerFactory("initial", [gameRouter]() { return std::make_unique<ForgeSplashScene>(gameRouter); });
    sceneRepository.registerFactory("menu", [gameRouter]() { return std::make_unique<ForgeMenuScene>(gameRouter); });
    sceneRepository.registerFactory(
        "game", [gameRouter, session]() { return std::make_unique<ForgeGameScene>(gameRouter, session); });
    sceneRepository.registerFactory("gameover", [gameRouter, session, records]() {
        return std::make_unique<ForgeGameOverScene>(gameRouter, session, records);
    });
    sceneRepository.registerFactory(
        "records", [gameRouter, records]() { return std::make_unique<ForgeRecordsScene>(gameRouter, records); });
    sceneRepository.registerFactory("exit", []() { return std::make_unique<ForgeExitScene>(); });
}
