#pragma once

#include <memory>

namespace cengine::routing {
class ISceneRepository;
}

class GameRouter;

// Registra as factories das cenas Forge no repositorio da cengine — a UNICA
// tabela codigo-de-estado -> cena da plataforma.
class ForgeSceneFactory
{
public:
    static void populateForgeScenes(cengine::routing::ISceneRepository& sceneRepository,
                                    const std::shared_ptr<GameRouter>&  gameRouter);
};
