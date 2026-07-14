// main_theforge.cpp — por onde tudo comeca: entry point da plataforma
// The-Forge em MODO BIBLIOTECA + composition root do jogo (mesmo padrao dos
// irmaos 8puzzle/spaceinvaders): o main vive fora da plataforma e do pacote
// do jogo, e e onde todas as instancias e injecoes principais acontecem.
//
// Primeiro consumidor do desenho 0.6.0 da cengine e do 0.1.0 do
// platform-theforge-common:
//
//   main()
//     -> initMemAlloc/initFileSystem/initLog   subsistemas de processo (na
//                                              ordem do WindowsMain)
//     -> composicao: repositorio de cenas -> router (estado inicial =
//        splash) -> GameRouter (FlowRouter da cengine + vocabulario) ->
//        factories de cena
//     -> EngineManager::owned(TheForgeWindowManager{desc}, GameManager)
//     -> engine.start()   loop da cengine: window.update() -> fases do jogo
//                         (input -> update(dt fixo) 0..N -> render) ->
//                         window.present(); shouldExit() -> cleanup()
//
// O dominio vive em src/asteroids/game/ e as cenas em
// src/platform/theforge/src/AsteroidsForge/scene/ — cenas so falam com
// forgeui (o batcher de sprites esta DESLIGADO neste degrau: atlasPath nulo).
//
// Este main NAO e um alvo CMake: compila no AsteroidsForge.vcxproj (MSBuild),
// que reutiliza a cadeia de build do The-Forge — dai os subsistemas de
// processo e os exports do Agility SDK aqui dentro.

// cengine + jogo — C++ puro, ANTES dos headers do The-Forge (IMemory.h por ultimo).
#include <cengine/core/EngineManager.hpp>
#include <cengine/routing/GameManager.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/SceneRepository.hpp>

#include "asteroids/game/GameRouter.h"
#include "asteroids/game/service/PlaySession.h"
#include "asteroids/game/state/StateGame.h"

#include "platform/theforge/src/AsteroidsForge/ForgeSceneFactory.h"

// platform-theforge-common (checkout irmao; include path no vcxproj)
#include "TheForgeWindowManager.h"

#include <memory>

#include "Common_3/OS/Interfaces/IOperatingSystem.h" // UINT dos exports do Agility
#include "Common_3/Utilities/Interfaces/IFileSystem.h"
#include "Common_3/Utilities/Interfaces/ILog.h"

#include "Common_3/Utilities/Interfaces/IMemory.h" // deve ser o ultimo include

// O DEFINE_APPLICATION_MAIN exportava estes simbolos para ativar o Agility
// SDK do D3D12 (D3D12Core.dll copiada para a pasta do exe). Sem IApp,
// exportamos por conta propria — D3D12_AGILITY_SDK_VERSION vem do
// TF_Shared.props.
extern "C"
{
    __declspec(dllexport) extern const UINT  D3D12SDKVersion = D3D12_AGILITY_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = "";
}

int main()
{
    constexpr const char* kAppName = "AsteroidsForge";

    // Subsistemas de processo na ordem do WindowsMain; o resto (janela, GPU,
    // fontes) e responsabilidade do TheForgeWindowManager, dentro da cengine.
    if (!initMemAlloc(kAppName))
        return EXIT_FAILURE;

    FileSystemInitDesc fsDesc = {};
    fsDesc.pAppName = kAppName;
    if (!initFileSystem(&fsDesc))
        return EXIT_FAILURE;

    initLog(kAppName, DEFAULT_LOG_LEVEL);

    {
        // composicao do jogo: quem conhece o grafo inteiro (dominio + cenas +
        // engine) e so este arquivo
        auto sceneRepository = std::make_unique<cengine::routing::SceneRepository>();
        cengine::routing::ISceneRepository& sceneRepositoryRef = *sceneRepository;

        const auto router =
            std::make_shared<cengine::routing::RouterInMemory>(std::move(sceneRepository), std::make_unique<InitialSG>());

        const auto gameRouter = std::make_shared<GameRouter>(router);

        // O resultado da partida atravessa a troca de cena (o World morre com a
        // cena do jogo); o PlaySession e quem sobrevive para o gameOver ler.
        const auto session = std::make_shared<PlaySession>();

        ForgeSceneFactory::populateForgeScenes(sceneRepositoryRef, gameRouter, session);

        // Casco do common: fonte do The-Forge, batcher de sprites DESLIGADO
        // (atlasPath nulo — casco so de texto neste degrau) e o preto-espaco
        // dos irmaos como cor de clear.
        TheForgeWindowDesc windowDesc = {};
        windowDesc.appName = kAppName;
        windowDesc.width = 1280;
        windowDesc.height = 720;
        windowDesc.fontPath = "TitilliumText/TitilliumText-Bold.otf";
        windowDesc.sprites.atlasPath = nullptr;

        // Modo PROPRIO por construcao (cengine 0.6.0): a cengine dirige o
        // loop e o The-Forge entra como biblioteca atras do IWindowManager.
        auto engine = cengine::core::EngineManager::owned(
            std::make_unique<TheForgeWindowManager>(windowDesc),
            std::make_unique<cengine::routing::GameManager>(router));

        engine.start(); // bloqueia ate o jogo rotear para "exit"; o
                        // cleanup() (jogo + janela/GPU) roda no fim do start()
    }

    LOGF(eINFO, "[asteroids] loop da cengine encerrado");

    exitLog();
    exitFileSystem();
    exitMemAlloc();
    return 0;
}
