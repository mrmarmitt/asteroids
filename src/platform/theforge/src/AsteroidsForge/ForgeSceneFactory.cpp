#include "ForgeSceneFactory.h"

#include <cengine/routing/ISceneRepository.hpp>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/PlaySession.h"

#include "scene/ForgeExitScene.h"
#include "scene/ForgeGameOverScene.h"
#include "scene/ForgeGameScene.h"
#include "scene/ForgeMenuScene.h"
#include "scene/ForgeSplashScene.h"

// As factories rodam LAZY (no primeiro getScene de cada estado) — capturas
// por VALOR. Estados que recriam a cena a cada visita (game) ganham partida
// zerada de graca; o PlaySession, que precisa ATRAVESSAR a troca de cena, e
// justamente o que vive fora delas.
void ForgeSceneFactory::populateForgeScenes(cengine::routing::ISceneRepository& sceneRepository,
                                            const std::shared_ptr<GameRouter>&  gameRouter,
                                            const std::shared_ptr<PlaySession>& session)
{
    sceneRepository.registerFactory("initial", [gameRouter]() { return std::make_unique<ForgeSplashScene>(gameRouter); });
    sceneRepository.registerFactory("menu", [gameRouter]() { return std::make_unique<ForgeMenuScene>(gameRouter); });
    sceneRepository.registerFactory("game",
                                    [gameRouter, session]() { return std::make_unique<ForgeGameScene>(gameRouter, session); });
    sceneRepository.registerFactory(
        "gameover", [gameRouter, session]() { return std::make_unique<ForgeGameOverScene>(gameRouter, session); });
    sceneRepository.registerFactory("exit", []() { return std::make_unique<ForgeExitScene>(); });
}
