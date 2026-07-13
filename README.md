# Asteroids

Jogo de estudo sobre a [cengine](https://github.com/mrmarmitt/cengine) com a
plataforma The-Forge via
[platform-theforge-common](https://github.com/mrmarmitt/platform-theforge-common).

Terceiro jogo do ecossistema (depois de 8puzzle e spaceinvaders — congelados
como documentacao viva, ADR 0003 da cengine) e o **consumidor de validacao**
das versoes novas:

- **cengine 0.6.0**: `EngineManager::owned()` (modo proprio, sem nullptr) e
  `FlowRouter<TFlow>` (mecanica da fachada de navegacao).
- **platform-theforge-common 0.1.0**: `TheForgeWindowManager`
  (`TheForgeWindowDesc`), `forgeui` (fila de edges + `isHeld`/`heldAxis`) e
  `forgesprite` (desligado no bootstrap — casco so de texto).

## Estrutura

```
src/main_theforge.cpp               entry point + composition root
src/asteroids/game/                 dominio (C++ puro, sem The-Forge)
  GameRouter.h                      fachada de navegacao (FlowRouter + vocabulario)
  state/StateGameFlow.h             maquina de fluxo (transicoes por estado)
  state/StateGame.h                 estados concretos (initial/menu/game/exit)
src/platform/theforge/
  PC_VS2019/AsteroidsForge.sln      build MSBuild (cadeia do The-Forge)
  src/AsteroidsForge/               cenas + factory + PathStatement + gpu.cfg
```

## Build (Windows, VS2019)

Pre-requisitos (checkouts irmaos na mesma pasta): `The-Forge` com a solution
`Examples_3/Unit_Tests` buildada na mesma Configuration, `cengine` >= 0.6.0 e
`platform-theforge-common` >= 0.1.0.

```
msbuild src\platform\theforge\PC_VS2019\AsteroidsForge.sln /p:Configuration=Debug /p:Platform=x64
out\theforge\x64\Debug\AsteroidsForge\AsteroidsForge.exe
```

## Controles (casco atual)

- Splash: ENTER entra no menu.
- Menu: SETAS navegam, ENTER confirma, ESC sai.
- Jogo (placeholder do casco): SETAS seguradas movem a nave, ESC volta ao menu.

## Plano

O plano de desenvolvimento vive em [`.ai/task/`](.ai/task/), comecando pelo
[bootstrap do casco](.ai/task/01-bootstrap-casco.md).
