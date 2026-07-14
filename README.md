# Asteroids

Jogo de estudo sobre a [cengine](https://github.com/mrmarmitt/cengine) com a
plataforma The-Forge via
[platform-theforge-common](https://github.com/mrmarmitt/platform-theforge-common).

Terceiro jogo do ecossistema (depois de 8puzzle e spaceinvaders — congelados
como documentacao viva, ADR 0003 da cengine) e o **consumidor de validacao**
das versoes novas:

A versao exata da cengine usada esta no **pin do `CMakeLists.txt`** (hoje
`0.7.1`) — e a unica fonte da verdade; o resto do repo fala das *capacidades*,
nao dos numeros, para nao apodrecer. O que este jogo validou:

- **cengine 0.6.0**: `EngineManager::owned()` (modo proprio, sem nullptr) e
  `FlowRouter<TFlow>` (mecanica da fachada de navegacao).
- **cengine 0.7.x**: `cengine::collision2d` — a DETECCAO circulo x circulo. O
  **wrap-around da arena fica aqui**, no jogo: o `World` corrige a posicao pelo
  menor delta no toro e so entao pergunta a engine, que nao opina sobre o
  formato do mundo.
- **platform-theforge-common 0.1.0**: `TheForgeWindowManager`
  (`TheForgeWindowDesc`), `forgeui` (fila de edges + `isHeld`/`heldAxis`) e
  `forgesprite` (desligado — a cena ainda desenha tudo com texto; a escolha do
  renderizador esta adiada, ver `.ai/task/02`).

## Estrutura

```
src/main_theforge.cpp               entry point + composition root
src/asteroids/game/                 dominio (C++ puro, sem The-Forge)
  World.{h,cpp}                     a simulacao: nave, tiros, rochas, ondas, colisao
  GameRouter.h                      fachada de navegacao (FlowRouter + vocabulario)
  state/StateGameFlow.h             maquina de fluxo (transicoes por estado)
  state/StateGame.h                 estados concretos (initial/menu/game/exit)
test/asteroids/game/WorldTest.cpp   a suite do dominio (GoogleTest/CTest)
src/platform/theforge/
  PC_VS2019/AsteroidsForge.sln      build MSBuild (cadeia do The-Forge)
  src/AsteroidsForge/               cenas + factory + PathStatement + gpu.cfg
```

O dominio nao inclui NADA do The-Forge: os testes exercitam o `World` do mesmo
jeito que a cena (comandos -> `update(dt fixo)` -> consultas), sem GPU.

## Build

Sao **dois builds independentes**, como nos jogos irmaos: o dominio (CMake, para
os testes) e o jogo (MSBuild, contra a arvore do The-Forge).

### Dominio + testes (CMake)

Presets portateis (sem caminho de maquina) vivem em `CMakePresets.json`:

| Preset | Proposito |
|--------|-----------|
| `debug` | Build de debug (Ninja) |
| `release` | Build de release (Ninja) |

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Presets especificos de maquina (caminhos de compilador, ex.: MSYS2/MinGW) vao
num `CMakeUserPresets.json` **nao versionado** (esta no `.gitignore`).

> **Nota MSYS2/MinGW:** os presets `debug`/`release` usam o gerador Ninja. O
> `ninja` que vem no MSYS2 roda os comandos atraves do `/bin/sh`, que destroi
> caminhos nativos do Windows (`C:\...` vira `C:msys64...`) e o build morre no
> primeiro teste de compilador. Nessas maquinas, use um preset proprio com o
> gerador `MinGW Makefiles` no seu `CMakeUserPresets.json` — ou instale um Ninja
> nativo. Mesma armadilha (e mesma nota) da cengine.

### Jogo (MSBuild, Windows)

Pre-requisitos (checkouts irmaos na mesma pasta): `The-Forge` com a solution
`Examples_3/Unit_Tests` buildada **na mesma Configuration**, `cengine` >= 0.7.0
e `platform-theforge-common` >= 0.1.0.

```
msbuild src\platform\theforge\PC_VS2019\AsteroidsForge.sln /p:Configuration=Release /p:Platform=x64
out\theforge\x64\Release\AsteroidsForge\AsteroidsForge.exe
```

> A Configuration tem de existir no The-Forge. Nesta maquina so a **Release**
> esta buildada — dai os exemplos usarem Release.

## Controles

- Splash: ENTER entra no menu.
- Menu: SETAS navegam, ENTER confirma, ESC sai.
- Jogo: SETAS `<-` `->` giram, SETA CIMA acelera (inercia!), ESPACO atira,
  ESC volta ao menu.

A arena da a volta: nave, tiros e rochas somem por uma borda e voltam pela
oposta.

## Plano

O plano de desenvolvimento vive em [`.ai/task/`](.ai/task/), um degrau validado
por vez. Feito ate aqui: casco, nave (rotacao/thrust/inercia/tiros) e rochas
(ondas, fragmentacao, colisao).
